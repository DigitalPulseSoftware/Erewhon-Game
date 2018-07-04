// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_CLIENT_STATES_FLEETEDITSTATE_HPP
#define EREWHON_CLIENT_STATES_FLEETEDITSTATE_HPP

#include <Client/States/AbstractState.hpp>
#include <Nazara/Graphics/Model.hpp>
#include <Nazara/Math/EulerAngles.hpp>
#include <Nazara/Platform/EventHandler.hpp>
#include <NDK/State.hpp>
#include <NDK/Widgets.hpp>
#include <hopstotch/hopscotch_map.h>
#include <vector>

namespace ewn
{
	class FleetEditState final : public AbstractState
	{
		public:
			inline FleetEditState(StateData& stateData, std::shared_ptr<Ndk::State> previousState, std::string fleetName);
			~FleetEditState() = default;

		private:
			struct Spaceship;
			struct SpaceshipData;

			void Enter(Ndk::StateMachine& fsm) override;
			void Leave(Ndk::StateMachine& fsm) override;
			bool Update(Ndk::StateMachine& fsm, float elapsedTime) override;

			void AddSpaceship(std::size_t spaceshipDataId, const Nz::Vector3f& position = Nz::Vector3f::Zero());

			void CreateGrid();

			inline bool IsAnySpaceshipInCollision() const;
			inline bool IsInEditMode() const;

			void LayoutWidgets() override;

			std::size_t RegisterSpaceshipData(std::string spaceshipName, const std::string& modelPath, float scale, const Nz::Boxf& collisionBox);

			void QueryFleetInfo();
			void QuerySpaceshipInfo(std::string spaceshipName);
			void QuerySpaceshipList();

			void SetupForCreate();
			void SetupForUpdate();

			void UpdateCollisions();
			void UpdateGrid();
			void UpdateStatus(const Nz::String& status, const Nz::Color& color = Nz::Color::White);

			// Input events
			void OnMouseButtonPressed(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::MouseButtonEvent& event);
			void OnMouseButtonReleased(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::MouseButtonEvent& event);
			void OnMouseMoved(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::MouseMoveEvent& event);
			void OnMouseWheelMoved(const Nz::EventHandler* eventHandler, const Nz::WindowEvent::MouseWheelEvent& event);

			// GUI Event
			void OnAddSpaceship();
			void OnBackPressed();
			void OnCreatePressed();
			void OnDeletePressed();
			void OnUpdatePressed();

			// Network event
			void OnCreateFleetFailure(ServerConnection* server, const Packets::CreateFleetFailure& createPacket);
			void OnCreateFleetSuccess(ServerConnection* server, const Packets::CreateFleetSuccess& createPacket);
			void OnDeleteFleetFailure(ServerConnection* server, const Packets::DeleteFleetFailure& deletePacket);
			void OnDeleteFleetSuccess(ServerConnection* server, const Packets::DeleteFleetSuccess& deletePacket);
			void OnFleetInfo(ServerConnection* server,          const Packets::FleetInfo& listPacket);
			void OnUpdateFleetFailure(ServerConnection* server, const Packets::UpdateFleetFailure& updatePacket);
			void OnUpdateFleetSuccess(ServerConnection* server, const Packets::UpdateFleetSuccess& updatePacket);
			void OnSpaceshipInfo(ServerConnection* server, const Packets::SpaceshipInfo& spaceshipInfo);
			void OnSpaceshipList(ServerConnection* server, const Packets::SpaceshipList& spaceshipList);

			static inline float Snap(float position);
			static inline Nz::Vector3f SnapToGrid(Nz::Vector3f position);

			enum class MovementType
			{
				None,
				Entity,
				Grid
			};

			struct Spaceship
			{
				Ndk::EntityOwner entity;
				Nz::Boxf collisionBox;
				Nz::Vector3f targetPosition;
				std::size_t dataId;
				bool hasCollisions;
			};

			struct SpaceshipData
			{
				Nz::Boxf collisionBox;
				Nz::ModelRef model;
				std::string spaceshipName;
				float scale;
			};

			static constexpr std::size_t NoEntity = std::numeric_limits<std::size_t>::max();
			static constexpr float GridScale = 0.5f;
			static constexpr unsigned int GridSize = 30;

			Ndk::ButtonWidget* m_backButton;
			Ndk::ButtonWidget* m_createUpdateButton;
			Ndk::ButtonWidget* m_deleteButton;
			Ndk::ButtonWidget* m_spaceshipAddButton;
			Ndk::LabelWidget* m_statusLabel;
			Ndk::LabelWidget* m_titleLabel;
			Ndk::LabelWidget* m_nameLabel;
			Ndk::LabelWidget* m_spaceshipAddLabel;
			Ndk::LabelWidget* m_spaceshipAddNamesLabel;
			Ndk::TextAreaWidget* m_nameTextArea;
			Ndk::TextAreaWidget* m_spaceshipAddTextArea;
			Ndk::EntityOwner m_hoveredEntity;
			Ndk::EntityOwner m_grid;
			Ndk::EntityOwner m_light;
			Nz::EulerAnglesf m_gridRotation;
			Nz::MaterialRef m_collisionMaterial;
			Nz::MaterialRef m_hoveredMaterial;
			std::shared_ptr<Ndk::State> m_previousState;
			std::size_t m_hoveredEntityId;
			std::size_t m_movingEntity;
			std::string m_fleetName;
			std::vector<Spaceship> m_spaceships;
			std::vector<SpaceshipData> m_spaceshipData;
			tsl::hopscotch_map<std::string, std::optional<std::size_t>> m_spaceshipNameToData;
			MovementType m_rotatingMode;
			bool m_deleteConfirmation;
			bool m_isInEditMode;
			float m_hoveredEntitySize;
			float m_gridDistance;
			float m_labelDisappearanceAccumulator;
	};
}

#include <Client/States/Game/FleetEditState.inl>

#endif // EREWHON_CLIENT_STATES_FLEETEDITSTATE_HPP
