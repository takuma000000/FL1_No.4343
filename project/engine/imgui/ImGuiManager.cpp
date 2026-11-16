#include "ImGuiManager.h"

#ifdef USE_IMGUI
#include "imgui.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#endif

namespace TKM {
	void ImGuiManager::Initialize(TKM::WindowsAPI* winApp, TKM::DirectXCommon* dxCommon) {
#ifdef USE_IMGUI
		HRESULT hr;
		dxCommon_ = dxCommon;
		winApp_ = winApp;

		//ImGuoのコンテキストを生成
		ImGui::CreateContext();
		// ImGuiドッキング
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		// ===============================
		// 日本語フォントの追加
		// ===============================
		io.Fonts->AddFontDefault(); // デフォルト英語フォント

		// 日本語フォントを読み込む
		ImFont* font = io.Fonts->AddFontFromFileTTF(
			"C:/Windows/Fonts/msmincho.ttc",   // Windows標準のメイリオ
			18.0f,                           // フォントサイズ
			nullptr,
			io.Fonts->GetGlyphRangesJapanese() // 日本語文字範囲
		);

		// 読み込み成功したら日本語フォントを既定に設定
		if (font) {
			io.FontDefault = font;
		} else {
			OutputDebugStringA("[ImGui] 日本語フォントの読み込みに失敗しました\\n");
		}

		///ImGuiの色設定場所===========================================
		//イチゴ色
		//SetColorStrawberry();

		// ホワイトタイガー色
		//SetColorWhiteTiger();

		// クリスタルガラス
		SetColorCrystalGlass();

		//レインボー キラキラ✨
		//SetColorRainbow();
		///===========================================================

		//Win32用の初期化
		ImGui_ImplWin32_Init(winApp_->GetHwnd());
		//デスクリプタ―ヒープ設定
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		//デスクリプタ―ヒープ
		hr = dxCommon_->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvHeap_));
		assert(SUCCEEDED(hr));

		// ImGuiのDirectX12初期化関数を呼び出す
		ImGui_ImplDX12_Init(
			dxCommon_->GetDevice(), // DirectX12デバイス
			static_cast<int>(dxCommon_->GetBackBufferCount()), // バックバッファの数（型をintにキャスト）
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, // バックバッファのフォーマット（RTVと一致させる必要あり）
			srvHeap_.Get(), // SRVヒープ（Shader Resource View用のディスクリプタヒープ）
			srvHeap_->GetCPUDescriptorHandleForHeapStart(), // SRVのCPU側のハンドル
			srvHeap_->GetGPUDescriptorHandleForHeapStart() // SRVのGPU側のハンドル
		);
#endif
	}

	void ImGuiManager::Finalize() {
#ifdef USE_IMGUI

		// ImGuiのDirectX12用の終了処理
		ImGui_ImplDX12_Shutdown();
		// ImGuiのWin32用の終了処理
		ImGui_ImplWin32_Shutdown();
		// ImGuiのコンテキストを破棄
		ImGui::DestroyContext();

		// デスクリプタヒープを解放
		srvHeap_.Reset();

#endif
	}

	void ImGuiManager::Begin() {
#ifdef USE_IMGUI
		//ImGuiフレーム開始
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
#endif
	}

	void ImGuiManager::End() {
#ifdef USE_IMGUI
		ImGui::Render();
#endif
	}

	void ImGuiManager::Draw() {
#ifdef USE_IMGUI
		// コマンドリストの取得
		ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

		//デスクリプタヒープの配列をセットするコマンド
		ID3D12DescriptorHeap* ppHeaps[] = { srvHeap_.Get() };
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		//描画コマンドを発行
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif
	}

	void ImGuiManager::SetColorStrawberry() {
#ifdef USE_IMGUI
		ImGuiStyle& style = ImGui::GetStyle();

		//========================================
		// スタイルの設定
		style.WindowRounding = 20.0f; // ウィンドウの角を丸くする
		style.FrameRounding = 4.0f;  // フレームの角を丸くする

		ImVec4* colors = style.Colors;

		// 🍓 ウィンドウとテキスト
		colors[ImGuiCol_Text] = ImVec4(0.98f, 0.8f, 0.3f, 1.0f);     // 種の黄色（いちごの種の明るさ）
		colors[ImGuiCol_WindowBg] = ImVec4(0.9f, 0.2f, 0.3f, 0.95f);  // 苺色の背景
		colors[ImGuiCol_Border] = ImVec4(1.0f, 0.98f, 0.95f, 1.0f);   // 白雪色の枠線（クリームっぽさ）

		// 🍓 フレーム（スライダーやボックスの背景）
		colors[ImGuiCol_FrameBg] = ImVec4(0.9f, 0.2f, 0.3f, 0.6f);   // 苺色
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.8f, 0.1f, 0.2f, 0.6f); // 完熟いちごの濃い赤
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.98f, 0.8f, 0.3f, 0.6f); // 種の黄色

		// 🍓 タイトルバー
		colors[ImGuiCol_TitleBg] = ImVec4(0.8f, 0.1f, 0.2f, 0.9f);  // 完熟いちごの濃い赤
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.98f, 0.8f, 0.3f, 0.9f); // 種の黄色（目立たせる）

		// 🍓 チェックボックス・スライダー
		colors[ImGuiCol_CheckMark] = ImVec4(0.3f, 0.7f, 0.3f, 1.0f);  // 若草色（いちごの葉の緑）
		colors[ImGuiCol_SliderGrab] = ImVec4(0.98f, 0.8f, 0.3f, 1.0f); // 種の黄色
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.8f, 0.1f, 0.2f, 1.0f); // 完熟いちごの濃い赤

		// 🍓 ボタン
		colors[ImGuiCol_Button] = ImVec4(0.98f, 0.8f, 0.3f, 0.7f);   // 種の黄色（ポップな印象）
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.9f, 0.2f, 0.3f, 0.7f); // 苺色
		colors[ImGuiCol_ButtonActive] = ImVec4(0.8f, 0.1f, 0.2f, 0.7f); // 完熟いちご

		// 🍓 ヘッダー（ツリーノードやタブ）
		colors[ImGuiCol_Header] = ImVec4(0.9f, 0.2f, 0.3f, 0.7f);   // 苺色
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.98f, 0.8f, 0.3f, 0.7f); // 種の黄色
		colors[ImGuiCol_HeaderActive] = ImVec4(0.8f, 0.1f, 0.2f, 0.7f); // 完熟いちご

		// 🍓 セパレーター（区切り線）
		colors[ImGuiCol_Separator] = ImVec4(1.0f, 0.98f, 0.95f, 0.7f);   // 白雪色（クリーム）

		// 🍓 タブ
		colors[ImGuiCol_Tab] = ImVec4(0.9f, 0.2f, 0.3f, 0.7f);     // 苺色
		colors[ImGuiCol_TabHovered] = ImVec4(0.98f, 0.8f, 0.3f, 0.7f); // 種の黄色
		colors[ImGuiCol_TabActive] = ImVec4(0.8f, 0.1f, 0.2f, 0.7f); // 完熟いちご

		// 🍓 ポップアップ（コンテキストメニュー）
		colors[ImGuiCol_PopupBg] = ImVec4(0.9f, 0.2f, 0.3f, 0.95f);  // 苺色
#endif
	}

	void ImGuiManager::SetColorWhiteTiger() {
#ifdef USE_IMGUI
		ImGuiStyle& style = ImGui::GetStyle();

		//========================================
		// スタイルの設定
		style.WindowRounding = 19.0f; // ウィンドウの角を丸くする
		style.FrameRounding = 4.0f;  // フレームの角を丸くする

		ImVec4* colors = style.Colors;

		// 🐯 **ウィンドウとテキスト**
		colors[ImGuiCol_Text] = ImVec4(0.7f, 0.9f, 1.0f, 1.0f);   // **ホワイトタイガーの目（水色の輝き）**
		colors[ImGuiCol_WindowBg] = ImVec4(0.35f, 0.35f, 0.35f, 0.95f);  // **グレーの背景（毛色）**
		colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);   // **黒い縞模様をイメージ（コントラストUP）**

		//— 🐯** スライダーやボックス（入力欄）**
		colors[ImGuiCol_FrameBg] = ImVec4(0.5f, 0.5f, 0.8f, 0.95f);  // **明るめのグレー（毛並み）**
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.0f, 0.0f, 1.0f, 1.0f); // **氷青（ホワイトタイガーの目）**
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.0f, 1.0f, 1.0f); // **さらに明るい水色**

		//— 🐯** タイトルバー**
		colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);  // **黒に近いグレー（縞模様イメージ）**
		colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // **ホワイトタイガーの目の水色**

		//— 🐯** チェックボックス・スライダーのつまみ**
		colors[ImGuiCol_CheckMark] = ImVec4(0.7f, 0.9f, 1.0f, 1.0f);  // **氷青（目の色）**
		colors[ImGuiCol_SliderGrab] = ImVec4(0.6f, 0.8f, 1.0f, 1.0f); // **明るい水色**
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.8f, 1.0f, 1.0f, 1.0f); // **ハイライト**

		//— 🐯** ボタン**
		colors[ImGuiCol_Button] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);   // **縞模様の黒に近いグレー**
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.6f, 0.7f, 0.8f, 1.0f); // **ホワイトタイガーの目**
		colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f); // **漆黒（力強さ）**

		//— 🐯** ヘッダー（タブ・ツリーノード）**
		colors[ImGuiCol_Header] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f); // **ダークグレー**
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.6f, 0.7f, 0.8f, 1.0f); // **ホワイトタイガーの目**
		colors[ImGuiCol_HeaderActive] = ImVec4(0.2f, 0.6f, 0.8f, 1.0f); // **アクティブ時に水色を強調**

		//— 🐯** 区切り線（セパレーター）**
		colors[ImGuiCol_Separator] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);   // **黒の縞模様をイメージ**

		//— 🐯** タブ**
		colors[ImGuiCol_Tab] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);     // **薄墨色**
		colors[ImGuiCol_TabHovered] = ImVec4(0.6f, 0.7f, 0.8f, 1.0f); // **ホワイトタイガーの目**
		colors[ImGuiCol_TabActive] = ImVec4(0.2f, 0.6f, 0.8f, 1.0f); // **アクティブ時は水色を強調**

		//— 🐯** ポップアップ（コンテキストメニュー）**
		colors[ImGuiCol_PopupBg] = ImVec4(0.4f, 0.4f, 0.4f, 0.95f);  // **背景に溶け込むグレー**
#endif
	}

	void ImGuiManager::SetColorRainbow() {
#ifdef USE_IMGUI
		ImGuiStyle& style = ImGui::GetStyle();

		//========================================
		// スタイルの設定
		style.WindowRounding = 19.0f; // ウィンドウの角を丸くする
		style.FrameRounding = 4.0f;  // フレームの角を丸くする

		ImVec4* colors = style.Colors; // 色配列へのポインタ取得

		// 🌈 **ウィンドウとテキスト**
		colors[ImGuiCol_Text] = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 1.0f, 1.0f, 0.95f);
		colors[ImGuiCol_Border] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);

		colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);

		colors[ImGuiCol_TitleBg] = ImVec4(0.5f, 0.0f, 1.0f, 1.0f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);

		colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);

		colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);

		colors[ImGuiCol_Header] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 1.0f, 0.5f, 1.0f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.5f, 0.0f, 1.0f, 1.0f);

		colors[ImGuiCol_Separator] = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);

		colors[ImGuiCol_Tab] = ImVec4(1.0f, 0.0f, 0.5f, 1.0f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.5f, 1.0f, 0.0f, 1.0f);
		colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.5f, 1.0f, 1.0f);

		colors[ImGuiCol_PopupBg] = ImVec4(1.0f, 0.5f, 1.0f, 0.95f);
#endif
	}
	void ImGuiManager::SetColorCrystalGlass() {
#ifdef USE_IMGUI
		ImGuiStyle& style = ImGui::GetStyle();

		//=============================================================
		// Crystal Glass
		// 透明なガラス、黒い凹凸、銀色の反射をイメージ
		//=============================================================

		// レイアウト
		style.WindowRounding = 7.0f;
		style.ChildRounding = 5.0f;
		style.FrameRounding = 4.0f;
		style.PopupRounding = 5.0f;
		style.ScrollbarRounding = 5.0f;
		style.GrabRounding = 3.0f;
		style.TabRounding = 4.0f;

		style.WindowBorderSize = 1.0f;
		style.ChildBorderSize = 1.0f;
		style.FrameBorderSize = 1.0f;
		style.PopupBorderSize = 1.0f;

		style.WindowPadding = ImVec2(14.0f, 12.0f);
		style.FramePadding = ImVec2(9.0f, 6.0f);
		style.ItemSpacing = ImVec2(9.0f, 8.0f);
		style.ItemInnerSpacing = ImVec2(7.0f, 5.0f);
		style.GrabMinSize = 12.0f;

		//=============================================================
		// カラーパレット
		//=============================================================

		const ImVec4 kBlackGlass = ImVec4(0.055f, 0.060f, 0.070f, 0.96f);
		const ImVec4 kSmokeGlass = ImVec4(0.105f, 0.115f, 0.130f, 0.97f);
		const ImVec4 kDarkSilver = ImVec4(0.190f, 0.205f, 0.225f, 1.00f);
		const ImVec4 kSilver = ImVec4(0.650f, 0.700f, 0.735f, 1.00f);
		const ImVec4 kIceWhite = ImVec4(0.900f, 0.940f, 0.965f, 1.00f);
		const ImVec4 kReflection = ImVec4(0.760f, 0.865f, 0.920f, 1.00f);

		ImVec4* colors = style.Colors;

		//=============================================================
		// テキスト
		//=============================================================

		colors[ImGuiCol_Text] = kIceWhite;
		colors[ImGuiCol_TextDisabled] =
			ImVec4(0.430f, 0.470f, 0.500f, 1.00f);

		//=============================================================
		// 背景
		//=============================================================

		colors[ImGuiCol_WindowBg] = kBlackGlass;

		colors[ImGuiCol_ChildBg] =
			ImVec4(0.085f, 0.095f, 0.110f, 0.92f);

		colors[ImGuiCol_PopupBg] = kSmokeGlass;

		//=============================================================
		// 枠線
		//=============================================================

		colors[ImGuiCol_Border] =
			ImVec4(0.540f, 0.600f, 0.640f, 0.55f);

		colors[ImGuiCol_BorderShadow] =
			ImVec4(0.000f, 0.000f, 0.000f, 0.00f);

		//=============================================================
		// タイトルバー
		//=============================================================

		colors[ImGuiCol_TitleBg] =
			ImVec4(0.090f, 0.100f, 0.115f, 1.00f);

		colors[ImGuiCol_TitleBgActive] =
			ImVec4(0.205f, 0.230f, 0.255f, 1.00f);

		colors[ImGuiCol_TitleBgCollapsed] =
			ImVec4(0.075f, 0.085f, 0.100f, 0.85f);

		//=============================================================
		// メニューバー
		//=============================================================

		colors[ImGuiCol_MenuBarBg] =
			ImVec4(0.115f, 0.130f, 0.145f, 0.98f);

		//=============================================================
		// 入力欄
		//=============================================================

		colors[ImGuiCol_FrameBg] =
			ImVec4(0.155f, 0.170f, 0.190f, 0.92f);

		colors[ImGuiCol_FrameBgHovered] =
			ImVec4(0.260f, 0.300f, 0.330f, 0.95f);

		colors[ImGuiCol_FrameBgActive] =
			ImVec4(0.350f, 0.410f, 0.455f, 0.98f);

		//=============================================================
		// ボタン
		//=============================================================

		colors[ImGuiCol_Button] = kDarkSilver;

		colors[ImGuiCol_ButtonHovered] =
			ImVec4(0.420f, 0.490f, 0.530f, 1.00f);

		colors[ImGuiCol_ButtonActive] = kSilver;

		//=============================================================
		// チェックボックス・スライダー
		//=============================================================

		colors[ImGuiCol_CheckMark] = kIceWhite;

		colors[ImGuiCol_SliderGrab] = kSilver;
		colors[ImGuiCol_SliderGrabActive] = kReflection;

		//=============================================================
		// ヘッダー・選択項目
		//=============================================================

		colors[ImGuiCol_Header] =
			ImVec4(0.240f, 0.270f, 0.300f, 0.85f);

		colors[ImGuiCol_HeaderHovered] =
			ImVec4(0.390f, 0.455f, 0.495f, 0.95f);

		colors[ImGuiCol_HeaderActive] =
			ImVec4(0.520f, 0.590f, 0.630f, 1.00f);

		//=============================================================
		// セパレーター
		//=============================================================

		colors[ImGuiCol_Separator] =
			ImVec4(0.490f, 0.540f, 0.570f, 0.50f);

		colors[ImGuiCol_SeparatorHovered] = kSilver;
		colors[ImGuiCol_SeparatorActive] = kIceWhite;

		//=============================================================
		// タブ
		//=============================================================

		colors[ImGuiCol_Tab] =
			ImVec4(0.130f, 0.150f, 0.170f, 1.00f);

		colors[ImGuiCol_TabHovered] =
			ImVec4(0.390f, 0.450f, 0.490f, 1.00f);

		colors[ImGuiCol_TabActive] =
			ImVec4(0.285f, 0.330f, 0.365f, 1.00f);

		colors[ImGuiCol_TabUnfocused] =
			ImVec4(0.100f, 0.115f, 0.130f, 1.00f);

		colors[ImGuiCol_TabUnfocusedActive] =
			ImVec4(0.195f, 0.225f, 0.250f, 1.00f);

		//=============================================================
		// スクロールバー
		//=============================================================

		colors[ImGuiCol_ScrollbarBg] =
			ImVec4(0.065f, 0.075f, 0.085f, 0.90f);

		colors[ImGuiCol_ScrollbarGrab] =
			ImVec4(0.300f, 0.335f, 0.360f, 0.90f);

		colors[ImGuiCol_ScrollbarGrabHovered] = kSilver;
		colors[ImGuiCol_ScrollbarGrabActive] = kReflection;

		//=============================================================
		// その他
		//=============================================================

		colors[ImGuiCol_ResizeGrip] =
			ImVec4(0.570f, 0.640f, 0.690f, 0.35f);

		colors[ImGuiCol_ResizeGripHovered] =
			ImVec4(0.700f, 0.780f, 0.825f, 0.75f);

		colors[ImGuiCol_ResizeGripActive] = kIceWhite;

		colors[ImGuiCol_PlotLines] = kSilver;
		colors[ImGuiCol_PlotLinesHovered] = kIceWhite;

		colors[ImGuiCol_PlotHistogram] = kSilver;
		colors[ImGuiCol_PlotHistogramHovered] = kReflection;

		colors[ImGuiCol_TextSelectedBg] =
			ImVec4(0.540f, 0.650f, 0.720f, 0.38f);

		colors[ImGuiCol_DragDropTarget] = kReflection;
		colors[ImGuiCol_NavHighlight] = kReflection;

		colors[ImGuiCol_DockingPreview] =
			ImVec4(0.650f, 0.760f, 0.820f, 0.38f);

		colors[ImGuiCol_DockingEmptyBg] =
			ImVec4(0.045f, 0.050f, 0.060f, 1.00f);

		colors[ImGuiCol_ModalWindowDimBg] =
			ImVec4(0.000f, 0.000f, 0.000f, 0.65f);

#endif
	}
}