#include "pch.h"

#include "Application.h"

#include "GameLamp/Core/Core.h"
#include "GameLamp/Core/Window.h"
#include "GameLamp/Event/ApplicationEvent.h"
#include "GameLamp/Platform/Windows/WindowsWindow.h"
#include "GameLamp/Core/Input.h"
#include "GameLamp/ImGui/ImGuiLayer.h"

#include "glad/glad.h"

namespace GameLamp {

	Application* Application::s_Instance = nullptr;

	Application::Application()
		: m_Window{std::unique_ptr<Window>(Window::create())}
	{
		GL_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window->setEventCallback(GL_BIND_EVENT_FN(Application::onEvent));

		m_ImGuiLayer = new ImGuiLayer();
		pushOverlay(m_ImGuiLayer);

		glGenVertexArrays(1, &m_VertexArray);
		glBindVertexArray(m_VertexArray);

		float vertices[3 * 3] = {
			-0.5f, -0.5f, 0.0f,
			0.5f, -0.5f, 0.0f,
			0.0f, 0.5f, 0.0f
		};

		m_VertexBuffer.reset(VertexBuffer::create(vertices, sizeof(vertices)));

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

		unsigned int indices[3] = { 0, 1, 2 };
		m_IndexBuffer.reset(IndexBuffer::create(indices, sizeof(indices) / sizeof(uint32_t)));


		std::string vertexSrc = R"(
			#version 330 core

			layout(location=0) in vec3 a_Position;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = vec4(a_Position, 1.0);
			}	

		)";

		std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 outputColor;

			in vec3 v_Position;

			void main()
			{
				outputColor = vec4(v_Position * 0.5 + 0.5, 1.0);
			}
		
		)";

		m_Shader = std::make_unique<Shader>(vertexSrc, fragmentSrc);
	}

	Application::~Application()
	{
	}

	void Application::run()
	{
		WindowResizeEvent e(640, 640);

		GL_CORE_INFO(e);

	    while (m_Running) {
			glClearColor(0, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			m_Shader->bind();
			glBindVertexArray(m_VertexArray);
			glDrawElements(GL_TRIANGLES, m_IndexBuffer->getCount(), GL_UNSIGNED_INT, nullptr);

			for (Layer* layer : m_LayerStack)
				layer->onUpdate();

			m_ImGuiLayer->begin();
			for (Layer* layer : m_LayerStack)
				layer->onImGuiRender();
			m_ImGuiLayer->end();

			m_Window->onUpdate();
		};
	}

	void Application::onEvent(Event& e)
	{
		EventDispatcher m_EventDispatcher(e);

		m_EventDispatcher.dispatch<WindowCloseEvent>(GL_BIND_EVENT_FN(Application::onWindowCloseEvent));
		m_EventDispatcher.dispatch<WindowResizeEvent>(GL_BIND_EVENT_FN(Application::onWindowResizeEvent));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled) break;
			(*it)->onEvent(e);
		}
	}

	void Application::pushLayer(Layer* layer)
	{
		m_LayerStack.pushLayer(layer);
	}

	void Application::pushOverlay(Layer* overlay)
	{
		m_LayerStack.pushOverlay(overlay);
	}

	bool Application::onWindowCloseEvent(WindowCloseEvent&)
	{
		m_Running = false;
		return true;
	}

	bool Application::onWindowResizeEvent(WindowResizeEvent& e)
	{
		return true;
	}

}  // namespace GameLamp
