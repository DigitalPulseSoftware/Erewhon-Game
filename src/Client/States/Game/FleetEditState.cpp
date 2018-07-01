// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/Game/FleetEditState.hpp>
#include <Nazara/Core/Primitive.hpp>
#include <Nazara/Core/String.hpp>
#include <Nazara/Math/Ray.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <Nazara/Utility/StaticMesh.hpp>
#include <Nazara/Utility/VertexMapper.hpp>
#include <NDK/Components/CameraComponent.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/LightComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <NDK/StateMachine.hpp>
#include <Shared/Utils.hpp>
#include <cassert>

namespace ewn
{
	void FleetEditState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();

		const ConfigFile& config = stateData.app->GetConfig();

		m_deleteConfirmation = false;
		m_gridDistance = 20.f;
		m_gridRotation = Nz::EulerAnglesf(30.f, 0.f, 0.f);
		m_hoveredEntityId = NoEntity;
		m_labelDisappearanceAccumulator = 0.f;
		m_rotatingMode = MovementType::None;

		m_collisionMaterial = Nz::Material::New();
		m_collisionMaterial->SetDiffuseColor(Nz::Color::Red);
		m_collisionMaterial->SetFaceFilling(Nz::FaceFilling_Line);

		m_hoveredMaterial = Nz::Material::New();
		m_hoveredMaterial->SetFaceCulling(Nz::FaceSide_Front);

		m_hoveredEntity = stateData.world3D->CreateEntity();
		m_hoveredEntity->AddComponent<Ndk::NodeComponent>();
		m_hoveredEntity->AddComponent<Ndk::GraphicsComponent>();
		m_hoveredEntity->Disable();

		m_backButton = CreateWidget<Ndk::ButtonWidget>();
		m_backButton->SetPadding(15.f, 15.f, 15.f, 15.f);
		m_backButton->UpdateText(Nz::SimpleTextDrawer::Draw("Back", 24));
		m_backButton->ResizeToContent();
		m_backButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnBackPressed();
		});

		m_deleteButton = CreateWidget<Ndk::ButtonWidget>();
		m_deleteButton->SetPadding(15.f, 15.f, 15.f, 15.f);
		m_deleteButton->UpdateText(Nz::SimpleTextDrawer::Draw("Delete fleet", 24));
		m_deleteButton->ResizeToContent();
		m_deleteButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnDeletePressed();
		});

		m_createUpdateButton = CreateWidget<Ndk::ButtonWidget>();
		m_createUpdateButton->SetPadding(15.f, 15.f, 15.f, 15.f);

		m_nameLabel = CreateWidget<Ndk::LabelWidget>();
		m_nameLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Fleet name:", 24));
		m_nameLabel->ResizeToContent();

		m_nameTextArea = CreateWidget<Ndk::TextAreaWidget>();
		m_nameTextArea->SetContentSize({ 160.f, 30.f });
		m_nameTextArea->EnableBackground(true);
		m_nameTextArea->SetBackgroundColor(Nz::Color::White);
		m_nameTextArea->SetTextColor(Nz::Color::Black);

		m_spaceshipAddLabel = CreateWidget<Ndk::LabelWidget>();
		m_spaceshipAddLabel->UpdateText(Nz::SimpleTextDrawer::Draw("Spaceship name:", 24));
		m_spaceshipAddLabel->ResizeToContent();

		m_spaceshipAddNamesLabel = CreateWidget<Ndk::LabelWidget>();

		m_spaceshipAddTextArea = CreateWidget<Ndk::TextAreaWidget>();
		m_spaceshipAddTextArea->SetContentSize({ 160.f, 30.f });
		m_spaceshipAddTextArea->EnableBackground(true);
		m_spaceshipAddTextArea->SetBackgroundColor(Nz::Color::White);
		m_spaceshipAddTextArea->SetTextColor(Nz::Color::Black);

		m_spaceshipAddButton = CreateWidget<Ndk::ButtonWidget>();
		m_spaceshipAddButton->SetPadding(15.f, 15.f, 15.f, 15.f);
		m_spaceshipAddButton->UpdateText(Nz::SimpleTextDrawer::Draw("Add spaceship", 18));
		m_spaceshipAddButton->ResizeToContent();
		m_spaceshipAddButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnAddSpaceship();
		});

		m_statusLabel = CreateWidget<Ndk::LabelWidget>();
		m_titleLabel = CreateWidget<Ndk::LabelWidget>();

		m_light = stateData.world3D->CreateEntity();
		m_light->AddComponent<Ndk::LightComponent>(Nz::LightType_Directional);
		auto& lightNode = m_light->AddComponent<Ndk::NodeComponent>();
		lightNode.SetParent(stateData.camera3D);

		CreateGrid();

		LayoutWidgets();

		ConnectSignal(stateData.server->OnCreateFleetFailure, this, &FleetEditState::OnCreateFleetFailure);
		ConnectSignal(stateData.server->OnCreateFleetSuccess, this, &FleetEditState::OnCreateFleetSuccess);
		ConnectSignal(stateData.server->OnDeleteFleetFailure, this, &FleetEditState::OnDeleteFleetFailure);
		ConnectSignal(stateData.server->OnDeleteFleetSuccess, this, &FleetEditState::OnDeleteFleetSuccess);
		//ConnectSignal(stateData.server->OnFleetInfo, this, &FleetEditState::OnFleetInfo);
		ConnectSignal(stateData.server->OnUpdateFleetFailure, this, &FleetEditState::OnUpdateFleetFailure);
		ConnectSignal(stateData.server->OnUpdateFleetSuccess, this, &FleetEditState::OnUpdateFleetSuccess);
		ConnectSignal(stateData.server->OnSpaceshipInfo, this, &FleetEditState::OnSpaceshipInfo);
		ConnectSignal(stateData.server->OnSpaceshipList, this, &FleetEditState::OnSpaceshipList);
		ConnectSignal(stateData.window->GetEventHandler().OnMouseButtonPressed, this, &FleetEditState::OnMouseButtonPressed);
		ConnectSignal(stateData.window->GetEventHandler().OnMouseButtonReleased, this, &FleetEditState::OnMouseButtonReleased);
		ConnectSignal(stateData.window->GetEventHandler().OnMouseMoved, this, &FleetEditState::OnMouseMoved);
		ConnectSignal(stateData.window->GetEventHandler().OnMouseWheelMoved, this, &FleetEditState::OnMouseWheelMoved);

		QuerySpaceshipList();

		if (!IsInEditMode())
		{
			// Create mode
			SetupForCreate();
		}
		else
		{
			// Update mode
			SetupForUpdate();
			QueryFleetInfo();
		}
	}

	void FleetEditState::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		m_light.Reset();
		m_grid.Reset();
		m_spaceships.clear();
	}

	bool FleetEditState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		StateData& stateData = GetStateData();

		m_labelDisappearanceAccumulator -= elapsedTime;
		if (m_labelDisappearanceAccumulator < 0.f)
			m_statusLabel->Show(false);

		for (auto& spaceship : m_spaceships)
		{
			auto& spaceshipNode = spaceship.entity->GetComponent<Ndk::NodeComponent>();

			spaceship.entity->GetComponent<Ndk::NodeComponent>().SetPosition(DampenedString(spaceshipNode.GetPosition(Nz::CoordSys_Local), spaceship.targetPosition, elapsedTime, 10.f));
		}

		if (m_hoveredEntity->IsEnabled())
		{
			auto& hoveredNode = m_hoveredEntity->GetComponent<Ndk::NodeComponent>();
			auto& hoveredGfx = m_hoveredEntity->GetComponent<Ndk::GraphicsComponent>();

			auto& cameraComp = stateData.camera3D->GetComponent<Ndk::CameraComponent>();
			float distance = cameraComp.GetFrustum().GetPlane(Nz::FrustumPlane_Near).Distance(hoveredNode.GetPosition());

			float distanceFactor = distance / 3000.f;
			float sizeFactor = 40.f / m_hoveredEntitySize;

			hoveredNode.SetScale(1.f + distanceFactor * sizeFactor);
		}

		return true;
	}

	void FleetEditState::AddSpaceship(const SpaceshipData& data)
	{
		StateData& stateData = GetStateData();

		auto& spaceship = m_spaceships.emplace_back();
		spaceship.entity = stateData.world3D->CreateEntity();
		spaceship.targetPosition = Nz::Vector3f::Zero();
		spaceship.collisionBox = data.collisionBox;

		auto& spaceshipNode = spaceship.entity->AddComponent<Ndk::NodeComponent>();
		spaceshipNode.SetPosition(spaceship.targetPosition);
		spaceshipNode.SetParent(m_grid);

		Nz::ModelRef myModel = Nz::Model::New(*data.model);

		spaceship.entity->AddComponent<Ndk::GraphicsComponent>().Attach(myModel, Nz::Matrix4f::Scale(Nz::Vector3f(data.scale)));

		UpdateCollisions();
	}

	void FleetEditState::CreateGrid()
	{
		StateData& stateData = GetStateData();

		if (!Nz::ModelLibrary::Has("FleetEditGrid"))
		{
			Nz::VertexDeclaration* declaration = Nz::VertexDeclaration::Get(Nz::VertexLayout_XYZ);

			unsigned int indexCount = GridSize * 2 * 2;
			unsigned int vertexCount = GridSize * 2 * 2;

			Nz::IndexBufferRef indexBuffer = Nz::IndexBuffer::New(vertexCount > std::numeric_limits<Nz::UInt16>::max(), indexCount, Nz::DataStorage_Hardware, 0U);
			Nz::VertexBufferRef vertexBuffer = Nz::VertexBuffer::New(declaration, vertexCount, Nz::DataStorage_Hardware, 0U);

			Nz::VertexMapper vertexMapper(vertexBuffer, Nz::BufferAccess_WriteOnly);

			Nz::SparsePtr<Nz::Vector3f> positionPtr = vertexMapper.GetComponentPtr<Nz::Vector3f>(Nz::VertexComponent_Position);

			float halfSize = float(GridSize) / 2.f;

			float pos = -halfSize + 0.5f;
			for (unsigned int i = 0; i < GridSize; ++i)
			{
				*positionPtr++ = Nz::Vector3f(pos, 0.f, -halfSize);
				*positionPtr++ = Nz::Vector3f(pos, 0.f, halfSize);
				pos += 1.f;
			}

			pos = -halfSize + 0.5f;
			for (unsigned int i = 0; i < GridSize; ++i)
			{
				*positionPtr++ = Nz::Vector3f(-halfSize, 0.f, pos);
				*positionPtr++ = Nz::Vector3f(halfSize, 0.f, pos);
				pos += 1.f;
			}

			vertexMapper.Unmap();

			Nz::IndexMapper indexMapper(indexBuffer, Nz::BufferAccess_WriteOnly);

			for (unsigned int i = 0; i < indexCount; ++i)
				indexMapper.Set(i, i);

			indexMapper.Unmap();

			Nz::MeshRef gridMesh = Nz::Mesh::New();
			gridMesh->CreateStatic();

			Nz::StaticMeshRef subMesh = Nz::StaticMesh::New(vertexBuffer, indexBuffer);
			subMesh->GenerateAABB();
			subMesh->SetMaterialIndex(0);
			subMesh->SetPrimitiveMode(Nz::PrimitiveMode_LineList);

			gridMesh->AddSubMesh(subMesh);
			gridMesh->SetMaterialCount(1);

			Nz::ModelRef gridModel = Nz::Model::New();
			gridModel->SetMesh(gridMesh);

			Nz::ModelLibrary::Register("FleetEditGrid", std::move(gridModel));
		}

		m_grid = stateData.world3D->CreateEntity();
		m_grid->AddComponent<Ndk::GraphicsComponent>().Attach(Nz::ModelLibrary::Get("FleetEditGrid"), Nz::Matrix4f::Scale(Nz::Vector3f(1.f / GridScale)));
		auto& gridNode = m_grid->AddComponent<Ndk::NodeComponent>();
		gridNode.SetParent(stateData.camera3D);
		gridNode.SetScale(GridScale);

		UpdateGrid();
	}

	void FleetEditState::LayoutWidgets()
	{
		Nz::Vector2f canvasSize = GetStateData().canvas->GetSize();

		m_backButton->SetPosition(20.f, canvasSize.y - m_backButton->GetSize().y - 20.f);
		m_deleteButton->SetPosition(canvasSize.x - m_deleteButton->GetSize().x - 20.f, canvasSize.y - m_deleteButton->GetSize().y - 20.f);

		// Left
		Nz::Vector2f cursor = canvasSize * Nz::Vector2f(0.05f, 0.1f);
		m_spaceshipAddLabel->SetPosition(cursor);
		cursor.y += m_spaceshipAddLabel->GetSize().y + 5.f;

		m_spaceshipAddTextArea->SetPosition(cursor);
		cursor.y += m_spaceshipAddTextArea->GetSize().y + 5.f;

		m_spaceshipAddButton->SetPosition(cursor);
		cursor.y += m_spaceshipAddButton->GetSize().y + 5.f;

		m_spaceshipAddNamesLabel->SetPosition(cursor);

		// Central
		m_statusLabel->CenterHorizontal();
		m_statusLabel->SetPosition(m_statusLabel->GetPosition().x, canvasSize.y * 0.1f);

		m_titleLabel->CenterHorizontal();
		m_titleLabel->SetPosition(m_titleLabel->GetPosition().x, canvasSize.y * 0.8f - m_titleLabel->GetSize().y / 2.f);

		float totalNameWidth = m_nameLabel->GetSize().x + 5.f + m_nameTextArea->GetSize().x;
		m_nameLabel->SetPosition(canvasSize.x / 2.f - totalNameWidth / 2.f, canvasSize.y * 0.8f - m_titleLabel->GetSize().y / 2.f);
		m_nameTextArea->SetPosition(canvasSize.x / 2.f - totalNameWidth / 2.f + m_nameLabel->GetSize().x, canvasSize.y * 0.8f - m_titleLabel->GetSize().y / 2.f);

		m_createUpdateButton->CenterHorizontal();
		m_createUpdateButton->SetPosition(m_createUpdateButton->GetPosition().x, m_nameTextArea->GetPosition().y + m_nameTextArea->GetSize().y + 20.f);
	}

	void FleetEditState::QueryFleetInfo()
	{
		m_titleLabel->Show(false);

		UpdateStatus("Loading " + m_fleetName + "...");

		/*Packets::QueryFleetInfo packet;
		packet.spaceshipName = m_fleetName;

		GetStateData().server->SendPacket(std::move(packet));*/
	}

	void FleetEditState::QuerySpaceshipInfo(std::string spaceshipName)
	{
		Packets::QuerySpaceshipInfo packet;
		packet.spaceshipName = std::move(spaceshipName);

		GetStateData().server->SendPacket(std::move(packet));
	}

	void FleetEditState::QuerySpaceshipList()
	{
		GetStateData().server->SendPacket(Packets::QuerySpaceshipList());
	}

	void FleetEditState::SetupForCreate()
	{
		StateData& stateData = GetStateData();

		m_createUpdateButton->UpdateText(Nz::SimpleTextDrawer::Draw("Create", 24));
		m_createUpdateButton->ResizeToContent();

		m_createUpdateButton->OnButtonTrigger.Clear();
		m_createUpdateButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnCreatePressed();
		});

		m_deleteButton->Show(false);

		m_fleetName.clear();

		LayoutWidgets();
	}

	void FleetEditState::SetupForUpdate()
	{
		StateData& stateData = GetStateData();

		m_createUpdateButton->UpdateText(Nz::SimpleTextDrawer::Draw("Update", 24));
		m_createUpdateButton->ResizeToContent();

		m_createUpdateButton->OnButtonTrigger.Clear();
		m_createUpdateButton->OnButtonTrigger.Connect([&](const Ndk::ButtonWidget* /*button*/)
		{
			OnUpdatePressed();
		});

		m_deleteConfirmation = false;
		m_deleteButton->Show(true);
		m_deleteButton->UpdateText(Nz::SimpleTextDrawer::Draw("Delete Fleet", 24));
		m_deleteButton->ResizeToContent();

		m_nameTextArea->SetText(m_fleetName);

		LayoutWidgets();
	}

	void FleetEditState::UpdateCollisions()
	{
		for (auto& spaceship : m_spaceships)
		{
			bool doesCollide = false;
			for (const auto& otherSpaceship : m_spaceships)
			{
				if (otherSpaceship.entity == spaceship.entity)
					continue;

				if (spaceship.collisionBox.Intersect(otherSpaceship.collisionBox))
				{
					doesCollide = true;
					break;
				}
			}

			if (spaceship.hasCollisions != doesCollide)
			{
				spaceship.hasCollisions = doesCollide;

				auto& entityGfx = spaceship.entity->GetComponent<Ndk::GraphicsComponent>();
				entityGfx.ForEachRenderable([&](const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& localMatrix, int renderOrder)
				{
					renderable->SetSkin((doesCollide) ? 1 : 0);
				});
			}
		}
	}

	void FleetEditState::UpdateGrid()
	{
		auto& gridNode = m_grid->GetComponent<Ndk::NodeComponent>();
		gridNode.SetPosition(Nz::Vector3f::Forward() * m_gridDistance);
		gridNode.SetRotation(Nz::EulerAnglesf(m_gridRotation.pitch, 0.f, 0.f));
		gridNode.Rotate(Nz::EulerAnglesf(0.f, m_gridRotation.yaw, 0.f));
	}

	void FleetEditState::UpdateStatus(const Nz::String& status, const Nz::Color& color)
	{
		m_statusLabel->Show(true);
		m_statusLabel->UpdateText(Nz::SimpleTextDrawer::Draw(status, 24, 0U, color));
		m_statusLabel->ResizeToContent();

		m_labelDisappearanceAccumulator = status.GetLength() / 10.f;

		LayoutWidgets();
	}

	void FleetEditState::OnMouseButtonPressed(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::MouseButtonEvent& event)
	{
		StateData& stateData = GetStateData();

		if (event.button == Nz::Mouse::Left)
		{
			if (m_hoveredEntityId == NoEntity)
				m_rotatingMode = MovementType::Grid;
			else
			{
				m_rotatingMode = MovementType::Entity;
				m_movingEntity = m_hoveredEntityId;
			}
		}
		else if (event.button == Nz::Mouse::Right)
		{
			auto& cameraComp = stateData.camera3D->GetComponent<Ndk::CameraComponent>();
			auto& gridNode = m_grid->GetComponent<Ndk::NodeComponent>();

			// Make all computations in grid referential
			Nz::Planef plane = Nz::Planef::XZ();

			Nz::Vector2f fMousePos(event.x, event.y);
			Nz::Vector3f nearPoint = cameraComp.Unproject(Nz::Vector3f(fMousePos.x, fMousePos.y, 0.f));
			Nz::Vector3f farPoint = cameraComp.Unproject(Nz::Vector3f(fMousePos.x, fMousePos.y, 1.f));

			Nz::Vector3f rayOrigin = gridNode.ToLocalPosition(nearPoint);
			Nz::Vector3f rayEnd = gridNode.ToLocalPosition(farPoint);
			Nz::Vector3f rayNormal = (rayEnd - rayOrigin).Normalize();

			Nz::Rayf ray(rayOrigin, rayNormal);

			float hitDist;
			if (ray.Intersect(plane, &hitDist))
			{
				Nz::MaterialRef cubeMat = Nz::Material::New();
				cubeMat->SetDiffuseColor(Nz::Color(std::rand() % 255, std::rand() % 255, std::rand() % 255));
				cubeMat->SetShader("PhongLighting");

				Nz::MeshRef cubeMesh = Nz::Mesh::New();
				cubeMesh->CreateStatic();
				cubeMesh->BuildSubMesh(Nz::Primitive::Box(Nz::Vector3f::Unit(), Nz::Vector3ui(0)));

				Nz::ModelRef cubeModel = Nz::Model::New();
				cubeModel->SetMesh(cubeMesh);
				cubeModel->SetSkinCount(2);

				cubeModel->SetMaterial(0, 0, cubeMat);
				cubeModel->SetMaterial(1, 0, m_collisionMaterial);

				auto& spaceship = m_spaceships.emplace_back();
				spaceship.entity = stateData.world3D->CreateEntity();
				spaceship.targetPosition = SnapToGrid(ray.GetPoint(hitDist));
				spaceship.targetPosition.y = 0.f;
				spaceship.collisionBox = Nz::Boxf(spaceship.targetPosition.x - 0.5f, spaceship.targetPosition .y - 0.5f, spaceship.targetPosition .z - 0.5f, 1.f, 1.f, 1.f);

				auto& testNode = spaceship.entity->AddComponent<Ndk::NodeComponent>();

				spaceship.entity->AddComponent<Ndk::GraphicsComponent>().Attach(cubeModel);
				testNode.SetPosition(spaceship.targetPosition);
				testNode.SetParent(m_grid);

				UpdateCollisions();
			}
		}
	}

	void FleetEditState::OnMouseButtonReleased(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::MouseButtonEvent& event)
	{
		if (event.button == Nz::Mouse::Left)
			m_rotatingMode = MovementType::None;
	}

	void FleetEditState::OnMouseMoved(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::MouseMoveEvent& event)
	{
		StateData& stateData = GetStateData();
		if (m_rotatingMode == MovementType::None)
		{
			auto& cameraComp = stateData.camera3D->GetComponent<Ndk::CameraComponent>();
			Nz::Vector2f fMousePos(event.x, event.y);
			Nz::Vector3f nearPoint = cameraComp.Unproject(Nz::Vector3f(fMousePos.x, fMousePos.y, 0.f));
			Nz::Vector3f farPoint = cameraComp.Unproject(Nz::Vector3f(fMousePos.x, fMousePos.y, 1.f));

			Nz::Vector3f rayNormal = (farPoint - nearPoint).Normalize();
			Nz::Rayf ray(nearPoint, rayNormal);

			float closestHitDistance = std::numeric_limits<float>::infinity();
			std::size_t closestHit = NoEntity;

			for (std::size_t i = 0; i < m_spaceships.size(); ++i)
			{
				const auto& spaceship = m_spaceships[i];
				auto& entityGfx = spaceship.entity->GetComponent<Ndk::GraphicsComponent>();

				const Nz::Boxf& aabb = entityGfx.GetBoundingVolume().aabb;

				float hitDistance;
				if (ray.Intersect(aabb, &hitDistance))
				{
					if (hitDistance < closestHitDistance)
					{
						closestHitDistance = hitDistance;
						closestHit = i;
					}
				}
			}

			if (closestHit != m_hoveredEntityId)
			{
				m_hoveredEntityId = closestHit;

				if (closestHit != NoEntity)
				{
					const Ndk::EntityHandle& entity = m_spaceships[closestHit].entity;

					auto& entityGfx = entity->GetComponent<Ndk::GraphicsComponent>();

					m_hoveredEntitySize = entityGfx.GetBoundingVolume().aabb.GetRadius();

					m_hoveredEntity->Enable();

					auto& hoveredNode = m_hoveredEntity->GetComponent<Ndk::NodeComponent>();
					hoveredNode.SetParent(entity);

					auto& hoveredGfx = m_hoveredEntity->GetComponent<Ndk::GraphicsComponent>();
					hoveredGfx.Clear();

					entity->GetComponent<Ndk::GraphicsComponent>().ForEachRenderable([&](const Nz::InstancedRenderableRef& renderable, const Nz::Matrix4f& localMatrix, int renderOrder)
					{
						if (std::unique_ptr<Nz::InstancedRenderable> clonedRenderable = renderable->Clone())
						{
							std::size_t materialCount = clonedRenderable->GetMaterialCount();
							for (std::size_t i = 0; i < materialCount; ++i)
								clonedRenderable->SetMaterial(i, m_hoveredMaterial);

							hoveredGfx.Attach(clonedRenderable.release(), localMatrix, -1);
						}
					});
				}
				else
					m_hoveredEntity->Disable();
			}
		}
		else if (m_rotatingMode == MovementType::Grid)
		{
			m_gridRotation.pitch = Nz::Clamp(m_gridRotation.pitch + event.deltaY, -90.f, 90.f);
			m_gridRotation.yaw = Nz::NormalizeAngle(m_gridRotation.yaw + event.deltaX);

			UpdateGrid();
		}
		else if (m_rotatingMode == MovementType::Entity)
		{
			auto& cameraComp = stateData.camera3D->GetComponent<Ndk::CameraComponent>();
			Nz::Planef plane = Nz::Planef::XZ();

			Nz::Vector2f fMousePos(event.x, event.y);
			Nz::Vector3f nearPoint = cameraComp.Unproject(Nz::Vector3f(fMousePos.x, fMousePos.y, 0.f));
			Nz::Vector3f farPoint = cameraComp.Unproject(Nz::Vector3f(fMousePos.x, fMousePos.y, 1.f));

			auto& gridNode = m_grid->GetComponent<Ndk::NodeComponent>();
			Nz::Vector3f rayOrigin = gridNode.ToLocalPosition(nearPoint);
			Nz::Vector3f rayEnd = gridNode.ToLocalPosition(farPoint);
			Nz::Vector3f rayNormal = (rayEnd - rayOrigin).Normalize();

			Nz::Rayf ray(rayOrigin, rayNormal);

			float hitDist;
			if (ray.Intersect(plane, &hitDist))
			{
				auto& spaceship = m_spaceships[m_movingEntity];
				spaceship.targetPosition = SnapToGrid(ray.GetPoint(hitDist));

				spaceship.collisionBox.x = spaceship.targetPosition.x - spaceship.collisionBox.width / 2.f;
				spaceship.collisionBox.y = spaceship.targetPosition.y - spaceship.collisionBox.height / 2.f;
				spaceship.collisionBox.z = spaceship.targetPosition.z - spaceship.collisionBox.depth / 2.f;

				UpdateCollisions();
			}
		}
	}

	void FleetEditState::OnMouseWheelMoved(const Nz::EventHandler* /*eventHandler*/, const Nz::WindowEvent::MouseWheelEvent& event)
	{
		if (m_hoveredEntityId == NoEntity)
		{
			m_gridDistance = Nz::Clamp(m_gridDistance - event.delta, 3.f, 50.f);
			UpdateGrid();
		}
		else
		{
			auto& spaceshipData = m_spaceships[m_hoveredEntityId];
			spaceshipData.targetPosition.y = Snap(Nz::Clamp(spaceshipData.targetPosition.y + event.delta, -10.f, 10.f));
			spaceshipData.collisionBox.y = spaceshipData.targetPosition.y - spaceshipData.collisionBox.height / 2.f;

			UpdateCollisions();
		}
	}

	void FleetEditState::OnAddSpaceship()
	{
		std::string spaceshipName = m_spaceshipAddTextArea->GetText().ToStdString();

		auto it = m_spaceshipNameToData.find(spaceshipName);
		if (it == m_spaceshipNameToData.end())
		{
			UpdateStatus("You have no spaceship named \"" + spaceshipName + "\"", Nz::Color::Red);
			return;
		}

		const std::optional<std::size_t>& spaceshipDataIndex = it->second;
		if (spaceshipDataIndex)
			AddSpaceship(m_spaceshipData[spaceshipDataIndex.value()]);
		else
			QuerySpaceshipInfo(spaceshipName);
	}

	void FleetEditState::OnBackPressed()
	{
		GetStateData().fsm->ChangeState(m_previousState);
	}

	void FleetEditState::OnCreatePressed()
	{
		assert(!IsInEditMode());

		Nz::String fleetName = m_nameTextArea->GetText();
		if (fleetName.IsEmpty())
		{
			UpdateStatus("Missing fleet name", Nz::Color::Red);
			return;
		}

		if (m_spaceships.empty())
		{
			UpdateStatus("You must have at least one spaceship in your fleet", Nz::Color::Red);
			return;
		}

		if (IsAnySpaceshipInCollision())
		{
			UpdateStatus("Some spaceships are colliding, move them before creating", Nz::Color::Red);
			return;
		}

		Packets::CreateFleet createFleet;
		createFleet.fleetName = fleetName.ToStdString();

		for (const auto& spaceship : m_spaceships)
		{
			const std::string& spaceshipName = m_spaceshipData[spaceship.dataId].spaceshipName;

			std::size_t nameId;
			auto it = std::find(createFleet.spaceshipNames.begin(), createFleet.spaceshipNames.end(), spaceshipName);
			if (it == createFleet.spaceshipNames.end())
			{
				nameId = createFleet.spaceshipNames.size();
				createFleet.spaceshipNames.push_back(spaceshipName);
			}
			else
				nameId = std::distance(createFleet.spaceshipNames.begin(), it);

			auto& packetData = createFleet.spaceships.emplace_back();
			packetData.spaceshipNameId = static_cast<Nz::UInt32>(nameId);
			packetData.spaceshipPosition = spaceship.targetPosition;
		}

		GetStateData().server->SendPacket(createFleet);
	}

	void FleetEditState::OnDeletePressed()
	{
		assert(IsInEditMode());

		if (!m_deleteConfirmation)
		{
			m_deleteButton->UpdateText(Nz::SimpleTextDrawer::Draw("Delete fleet\n(confirm)", 24));
			m_deleteButton->ResizeToContent();

			m_deleteConfirmation = true;

			LayoutWidgets();
			return;
		}

		m_deleteConfirmation = false;

		Packets::DeleteFleet deleteFleet;
		deleteFleet.fleetName = m_fleetName;

		GetStateData().server->SendPacket(deleteFleet);
	}

	void FleetEditState::OnUpdatePressed()
	{
		assert(IsInEditMode());

		if (m_spaceships.empty())
		{
			UpdateStatus("You must have at least one spaceship in your fleet", Nz::Color::Red);
			return;
		}

		if (IsAnySpaceshipInCollision())
		{
			UpdateStatus("Some spaceships are colliding, move them before updating", Nz::Color::Red);
			return;
		}

		Packets::UpdateFleet updateFleet;
		updateFleet.fleetName = m_fleetName;
		updateFleet.newFleetName = m_nameTextArea->GetText().ToStdString();
		if (updateFleet.newFleetName == updateFleet.fleetName)
			updateFleet.newFleetName.clear(); //< Don't send new-name

		for (const auto& spaceship : m_spaceships)
		{
			const std::string& spaceshipName = m_spaceshipData[spaceship.dataId].spaceshipName;

			std::size_t nameId;
			auto it = std::find(updateFleet.spaceshipNames.begin(), updateFleet.spaceshipNames.end(), spaceshipName);
			if (it == updateFleet.spaceshipNames.end())
			{
				nameId = updateFleet.spaceshipNames.size();
				updateFleet.spaceshipNames.push_back(spaceshipName);
			}
			else
				nameId = std::distance(updateFleet.spaceshipNames.begin(), it);

			auto& packetData = updateFleet.spaceships.emplace_back();
			packetData.spaceshipNameId = static_cast<Nz::UInt32>(nameId);
			packetData.spaceshipPosition = spaceship.targetPosition;
		}

		GetStateData().server->SendPacket(updateFleet);
	}

	void FleetEditState::OnCreateFleetFailure(ServerConnection* server, const Packets::CreateFleetFailure& createPacket)
	{
		std::string reason;
		switch (createPacket.reason)
		{
			case CreateFleetFailureReason::AlreadyExists:
				reason = "fleet name is already taken";
				break;

			case CreateFleetFailureReason::ServerError:
				reason = "server error, please try again later";
				break;

			default:
				reason = "<packet error>";
				break;
		}

		UpdateStatus("Failed to create fleet: " + reason, Nz::Color::Red);
	}

	void FleetEditState::OnCreateFleetSuccess(ServerConnection* server, const Packets::CreateFleetSuccess& createPacket)
	{
		UpdateStatus("Fleet successfully created", Nz::Color::Green);

		m_fleetName = m_nameTextArea->GetText().ToStdString();
		m_isInEditMode = true;

		SetupForUpdate();
	}

	void FleetEditState::OnDeleteFleetFailure(ServerConnection* server, const Packets::DeleteFleetFailure& deletePacket)
	{
		std::string reason;
		switch (deletePacket.reason)
		{
			case DeleteFleetFailureReason::NotFound:
				reason = "fleet not found";
				break;

			case DeleteFleetFailureReason::ServerError:
				reason = "server error, please try again later";
				break;

			default:
				reason = "<packet error>";
				break;
		}

		UpdateStatus("Failed to delete fleet: " + reason, Nz::Color::Red);
	}

	void FleetEditState::OnDeleteFleetSuccess(ServerConnection* server, const Packets::DeleteFleetSuccess& deletePacket)
	{
		UpdateStatus("Fleet successfully deleted", Nz::Color::Green);

		m_isInEditMode = false;
		SetupForCreate();
	}

	/*void FleetEditState::OnFleetInfo(ServerConnection* server, const Packets::FleetInfo& infoPacket)
	{
		
		//LayoutWidgets();
	}*/

	void FleetEditState::OnUpdateFleetFailure(ServerConnection* server, const Packets::UpdateFleetFailure& updatePacket)
	{
		std::string reason;
		switch (updatePacket.reason)
		{
			case UpdateFleetFailureReason::NotFound:
				reason = "fleet not found";
				break;

			case UpdateFleetFailureReason::ServerError:
				reason = "server error, please try again later";
				break;

			default:
				reason = "<packet error>";
				break;
		}

		UpdateStatus("Failed to update fleet: " + reason, Nz::Color::Red);
	}

	void FleetEditState::OnUpdateFleetSuccess(ServerConnection* server, const Packets::UpdateFleetSuccess& updatePacket)
	{
		UpdateStatus("Fleet successfully updated", Nz::Color::Green);

		m_fleetName = m_nameTextArea->GetText().ToStdString();
	}

	void FleetEditState::OnSpaceshipInfo(ServerConnection* /*server*/, const Packets::SpaceshipInfo& spaceshipInfo)
	{
		StateData& stateData = GetStateData();

		if (spaceshipInfo.spaceshipName.empty())
		{
			UpdateStatus("An error occurred", Nz::Color::Red);
			return;
		}

		auto it = m_spaceshipNameToData.find(spaceshipInfo.spaceshipName);
		assert(it != m_spaceshipNameToData.end());

		const std::string& assetsFolder = stateData.app->GetConfig().GetStringOption("AssetsFolder");
		std::string filePath = assetsFolder + '/' + spaceshipInfo.hullModelPath;

		Nz::ModelRef originalModel = Nz::ModelManager::Get(filePath);

		it.value().emplace(m_spaceshipData.size());
		SpaceshipData& spaceshipData = m_spaceshipData.emplace_back();
		spaceshipData.collisionBox = spaceshipInfo.collisionBox;
		spaceshipData.spaceshipName = spaceshipInfo.spaceshipName;
		spaceshipData.scale = spaceshipInfo.scale;

		// Duplicate model to add collision skin
		spaceshipData.model = Nz::Model::New(*originalModel);
		spaceshipData.model->SetSkinCount(2);

		for (std::size_t i = 0; i < spaceshipData.model->GetMaterialCount(); ++i)
			spaceshipData.model->SetMaterial(1, i, m_collisionMaterial);

		AddSpaceship(spaceshipData);
	}

	void FleetEditState::OnSpaceshipList(ServerConnection* /*server*/, const Packets::SpaceshipList& spaceshipList)
	{
		std::string allNames = "Available spaceships:";

		m_spaceshipNameToData.clear();
		m_spaceshipNameToData.reserve(spaceshipList.spaceships.size());
		for (auto& spaceshipInfo : spaceshipList.spaceships)
		{
			allNames += "\n";
			allNames += "- ";
			allNames += spaceshipInfo.name;
			m_spaceshipNameToData.emplace(std::move(spaceshipInfo.name), std::optional<std::size_t>());
		}

		m_spaceshipAddNamesLabel->UpdateText(Nz::SimpleTextDrawer::Draw(allNames, 16));
		m_spaceshipAddNamesLabel->ResizeToContent();

		LayoutWidgets();
	}
}
