#pragma once
#include "Vector2.h"
#include <algorithm>

namespace TKM {
	// ゲーム解像度は固定し、表示領域へのフィットと座標の往復変換だけを行う。
	struct GameViewMapping {
		Vector2 origin{}, size{};
		Vector2 resolution{ 1280.0f, 720.0f };
		static GameViewMapping Fit(Vector2 origin, Vector2 available, Vector2 resolution) {
			GameViewMapping result{ origin, {}, resolution };
			if (available.x <= 0 || available.y <= 0 || resolution.x <= 0 || resolution.y <= 0) { return result; }
			const float scale = (std::min)(available.x / resolution.x, available.y / resolution.y);
			result.size = { resolution.x * scale, resolution.y * scale };
			result.origin = { origin.x + (available.x - result.size.x) * 0.5f, origin.y + (available.y - result.size.y) * 0.5f };
			return result;
		}
		bool Valid() const { return size.x > 0 && size.y > 0; }
		Vector2 ToGame(Vector2 screen) const {
			if (!Valid()) { return {}; }
			return { (screen.x - origin.x) * resolution.x / size.x, (screen.y - origin.y) * resolution.y / size.y };
		}
		Vector2 ToScreen(Vector2 game) const {
			return { origin.x + game.x * size.x / resolution.x, origin.y + game.y * size.y / resolution.y };
		}
	};
}

#ifdef USE_IMGUI
#include "imgui.h"
namespace TKM {
	class DebugGameView {
	public:
#if defined(_DEBUG)
		inline static GameViewMapping mapping_{};
		inline static ImDrawList* drawList_ = nullptr;
		inline static bool hovered_ = false;
		static bool CanUseMouse() { return mapping_.Valid() && hovered_ && !ImGui::GetIO().WantTextInput && !ImGui::IsAnyItemActive(); }
		static ImVec2 MousePosition() {
			const auto mouse = ImGui::GetMousePos();
			const auto game = mapping_.ToGame({ mouse.x, mouse.y }); return { game.x, game.y };
		}
		static Vector2 ToScreen(Vector2 game) { return mapping_.ToScreen(game); }
		static ImDrawList* DrawList() { return drawList_; }
		static float GameUnitsPerPixel() { return mapping_.Valid() ? mapping_.resolution.x / mapping_.size.x : 1.0f; }
		static void PushClip() {
			drawList_->PushClipRect({ mapping_.origin.x, mapping_.origin.y },
				{ mapping_.origin.x + mapping_.size.x, mapping_.origin.y + mapping_.size.y }, true);
		}
		static void PopClip() { drawList_->PopClipRect(); }
#else
		static bool CanUseMouse() { return !ImGui::GetIO().WantCaptureMouse; }
		static ImVec2 MousePosition() { return ImGui::GetMousePos(); }
		static Vector2 ToScreen(Vector2 game) { return game; }
		static ImDrawList* DrawList() { return ImGui::GetBackgroundDrawList(); }
		static float GameUnitsPerPixel() { return 1.0f; }
		static void PushClip() {}
		static void PopClip() {}
#endif
	};
}
#endif
