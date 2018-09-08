// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Client" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Client/States/ConnectionLostState.hpp>
#include <Nazara/Utility/SimpleTextDrawer.hpp>
#include <NDK/StateMachine.hpp>
#include <NDK/Components/GraphicsComponent.hpp>
#include <NDK/Components/NodeComponent.hpp>
#include <Client/States/LoginState.hpp>
#include <cassert>

namespace ewn
{
	void ConnectionLostState::Enter(Ndk::StateMachine& fsm)
	{
		AbstractState::Enter(fsm);

		StateData& stateData = GetStateData();

		m_accumulator = 0.f;
		m_statusSprite = Nz::TextSprite::New();

		m_statusText = stateData.world2D->CreateEntity();
		m_statusText->AddComponent<Ndk::NodeComponent>();

		Ndk::GraphicsComponent& graphicsComponent = m_statusText->AddComponent<Ndk::GraphicsComponent>();
		graphicsComponent.Attach(m_statusSprite);

		UpdateStatus("Connection lost.");
	}

	void ConnectionLostState::Leave(Ndk::StateMachine& fsm)
	{
		AbstractState::Leave(fsm);

		m_statusSprite.Reset();
		m_statusText.Reset();
	}

	bool ConnectionLostState::Update(Ndk::StateMachine& fsm, float elapsedTime)
	{
		constexpr float quitGameAfter = 5.f;

		m_accumulator += elapsedTime;
		if (m_accumulator >= quitGameAfter)
			fsm.ChangeState(std::make_shared<ewn::LoginState>(GetStateData()));

		return true;
	}

	void ConnectionLostState::LayoutWidgets()
	{
		Ndk::GraphicsComponent& graphicsComponent = m_statusText->GetComponent<Ndk::GraphicsComponent>();
		Ndk::NodeComponent& nodeComponent = m_statusText->GetComponent<Ndk::NodeComponent>();

		Nz::Boxf textBox = graphicsComponent.GetAABB();
		Nz::Vector2ui windowSize = GetStateData().window->GetSize();
		nodeComponent.SetPosition(windowSize.x / 2 - textBox.width / 2, windowSize.y / 2 - textBox.height / 2);
	}

	void ConnectionLostState::UpdateStatus(const Nz::String& status, const Nz::Color& color, bool center)
	{
		assert(m_statusSprite);
		m_statusSprite->Update(Nz::SimpleTextDrawer::Draw(status, 24, 0U, color));

		if (center)
			LayoutWidgets();
	}
}
