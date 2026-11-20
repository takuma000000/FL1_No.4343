#define NOMINMAX
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "d3dx12.h"
#include <algorithm>
#include <cassert>

namespace TKM {
	void DirectXCommon::InitializeComicParameters() {
		if (comicConstantBuffer_) { return; }
		comicConstantBuffer_ = CreateBufferResource(sizeof(ComicParameters));
		const HRESULT hr = comicConstantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&comicMappedData_));
		assert(SUCCEEDED(hr));
		*comicMappedData_ = { MyMath::MakeIdentity4x4(), { 0, 0, 0, 1 }, 1, 0.015f, 3, 1, 0, 1, {} };
	}

	void DirectXCommon::SetComicProjection(const Matrix4x4& projection) {
		InitializeComicParameters();
		auto matrix = projection;
		comicMappedData_->projectionInverse = MyMath::Inverse4x4(matrix);
	}

	void DirectXCommon::SetComicParam(const Vector4& color, float width, float threshold, int steps, float blend, bool toon, float outlineStrength) {
		InitializeComicParameters();
		comicToonRequested_ = toon;
		comicMappedData_->lineColor = color;
		comicMappedData_->lineWidth = std::clamp(width, 1.0f, 5.0f);
		comicMappedData_->depthThreshold = std::clamp(threshold, 0.001f, 0.1f);
		comicMappedData_->shadeSteps = std::clamp(steps, 2, 8);
		comicMappedData_->toonBlend = std::clamp(blend, 0.0f, 1.0f);
		comicMappedData_->outlineStrength = std::clamp(outlineStrength, 0.0f, 1.0f);
		comicMappedData_->toonEnabled = postEffectPreview_ == PostEffectPreview::Comic && toon ? 1u : 0u;
	}

	void DirectXCommon::BindComicLighting(UINT rootParameterIndex) {
		InitializeComicParameters();
		// 通常表示へ戻ったら、同じObject3dシェーダーの分岐で元の明暗を使う。
		comicMappedData_->toonEnabled = postEffectPreview_ == PostEffectPreview::Comic && comicToonRequested_ ? 1u : 0u;
		commandList_->SetGraphicsRootConstantBufferView(rootParameterIndex, comicConstantBuffer_->GetGPUVirtualAddress());
	}

	void DirectXCommon::InitializeComicPipeline() {
		if (comicPipelineState_) { return; }
		InitializeComicParameters();
		const auto vertex = CompileShader(L"resources/shaders/CopyImage.VS.hlsl", L"vs_6_0");
		const auto pixel = CompileShader(L"resources/shaders/Comic.PS.hlsl", L"ps_6_0");
		CD3DX12_DESCRIPTOR_RANGE sceneRange, depthRange;
		sceneRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		depthRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);
		CD3DX12_ROOT_PARAMETER parameters[3];
		parameters[0].InitAsDescriptorTable(1, &sceneRange, D3D12_SHADER_VISIBILITY_PIXEL);
		parameters[1].InitAsDescriptorTable(1, &depthRange, D3D12_SHADER_VISIBILITY_PIXEL);
		parameters[2].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
		D3D12_STATIC_SAMPLER_DESC sampler{};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		CD3DX12_ROOT_SIGNATURE_DESC signatureDesc;
		signatureDesc.Init(_countof(parameters), parameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		Microsoft::WRL::ComPtr<ID3DBlob> signature, error;
		HRESULT hr = D3D12SerializeRootSignature(&signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		assert(SUCCEEDED(hr));
		hr = device_->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&comicRootSignature_));
		assert(SUCCEEDED(hr));
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline{};
		pipeline.pRootSignature = comicRootSignature_.Get();
		pipeline.VS = { vertex->GetBufferPointer(), vertex->GetBufferSize() };
		pipeline.PS = { pixel->GetBufferPointer(), pixel->GetBufferSize() };
		pipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		pipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		pipeline.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		pipeline.DepthStencilState.DepthEnable = FALSE;
		pipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		pipeline.DepthStencilState.StencilEnable = FALSE;
		pipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		pipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipeline.NumRenderTargets = 1;
		pipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		pipeline.SampleDesc.Count = 1;
		hr = device_->CreateGraphicsPipelineState(&pipeline, IID_PPV_ARGS(&comicPipelineState_));
		assert(SUCCEEDED(hr));

		// 深度リソースはtypeless、既存のDSVはD24、ここでは深度面だけをR24で読む。
		comicDepthSrvIndex_ = srvManager_->Allocate();
		D3D12_SHADER_RESOURCE_VIEW_DESC depthView{};
		depthView.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		depthView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		depthView.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		depthView.Texture2D.MipLevels = 1;
		device_->CreateShaderResourceView(depthStencilResource_.Get(), &depthView, srvManager_->GetCPUDescriptorHandle(comicDepthSrvIndex_));
	}

	void DirectXCommon::ApplyComic(ID3D12Resource* input, uint32_t inputSrv, ID3D12Resource* output, D3D12_CPU_DESCRIPTOR_HANDLE outputRtv) {
		assert(input && output && input != output && comicPipelineState_);
		// 深度を読む前にDSVを外す。前面UIの描画までに必ずDEPTH_WRITEへ戻す。
		commandList_->OMSetRenderTargets(1, &outputRtv, false, nullptr);
		D3D12_RESOURCE_BARRIER barriers[2] = {
			CD3DX12_RESOURCE_BARRIER::Transition(input, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
			CD3DX12_RESOURCE_BARRIER::Transition(depthStencilResource_.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
		};
		commandList_->ResourceBarrier(2, barriers);
		commandList_->SetGraphicsRootSignature(comicRootSignature_.Get());
		commandList_->SetPipelineState(comicPipelineState_.Get());
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		srvManager_->PreDraw();
		srvManager_->SetGraphicsRootDescriptorTable(0, inputSrv);
		srvManager_->SetGraphicsRootDescriptorTable(1, comicDepthSrvIndex_);
		commandList_->SetGraphicsRootConstantBufferView(2, comicConstantBuffer_->GetGPUVirtualAddress());
		commandList_->DrawInstanced(3, 1, 0, 0);
		for (auto& barrier : barriers) { std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter); }
		commandList_->ResourceBarrier(2, barriers);
	}
}
