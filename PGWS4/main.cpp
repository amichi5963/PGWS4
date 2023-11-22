#include <Windows.h> 
#include <tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<vector>
#include<d3dcompiler.h>
#ifdef _DEBUG
#include <iostream>
#endif

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
using namespace std;
using namespace DirectX;
// @brief コンソール画面にフォーマット付き文字列を表示
// @param format フォーマット (%d とか %f とかの)
// @param
// @remarks この関数はデバッグ用です。 デバッグ時にしか動作しません 
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}

// 面倒だけど書かなければいけない関数
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// ウィンドウが破棄されたら呼ばれる

	if (msg == WM_DESTROY) {
		PostQuitMessage(0); // OS に対して「もうこのアプリは終わる」と伝える 
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam); // 既定の処理を行う
}

#ifdef _DEBUG
int main()
{
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
}
#endif
const unsigned int window_width = 1280;
const unsigned int window_height = 720;

ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;
ID3D12CommandQueue* _cmdQueue = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

// ウィンドウクラスの生成&登録 
WNDCLASSEX w = {};

w.cbSize = sizeof(WNDCLASSEX);
w.lpfnWndProc = (WNDPROC)WindowProcedure; // コールバック関数の指定 
w.lpszClassName = _T("DX12Sample"); // アプリケーションクラス名 (適当でよい) 
w.hInstance = GetModuleHandle(nullptr); // ハンドルの取得

RegisterClassEx(&w); // アプリケーションクラス(ウィンドウクラスの指定を OS に伝える)

RECT wrc = { 0,0, window_width, window_height }; // ウィンドウサイズを決める

// 関数を使ってウィンドウのサイズを補正する 
AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
// ウィンドウオブジェクトの生成
HWND hwnd = CreateWindow(w.lpszClassName, // クラス名指定 
	_T("DX12 テスト"), // タイトルバーの文字
	WS_OVERLAPPEDWINDOW, // タイトルバーと境界線があるウィンドウ
	CW_USEDEFAULT, // 表示 x 座標は OS にお任せ
	CW_USEDEFAULT, // 表示 y 座標はOS にお任せ
	wrc.right - wrc.left, // ウィンドウ幅
	wrc.bottom - wrc.top, // ウィンドウ高
	nullptr, // 親ウィンドウハンドル
	nullptr, // メニューハンドル
	w.hInstance, // 呼び出しアプリケーションハンドル 
	nullptr); // 追加パラメーター

HRESULT D3D12CreateDevice(
	IUnknown* pAdapter,
	D3D_FEATURE_LEVEL MinimumFeatureLevel,
	REFIID riid,
	void** ppDwvice
);

D3D_FEATURE_LEVEL levels[] = {
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
};

auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
//アダプターの列挙用
std::vector<IDXGIAdapter*>adapters;
//ここに特定の名前を持つアダプターオブジェクトが入る
IDXGIAdapter* tmpAdapter = nullptr;
for (int i = 0;
	_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
	++i)
{
	adapters.push_back(tmpAdapter);
}
for (auto adpt : adapters) {
	DXGI_ADAPTER_DESC adesc = {};
	adpt->GetDesc(&adesc);//アダプターの説明オブジェクト取得
	std::wstring strDesc = adesc.Description;
	//探したいアダプターの名前を確認
	if (strDesc.find(L"NVIDIA") != std::string::npos)
	{
		tmpAdapter = adpt;
		break;
	}
}

//Direct3D デバイスの初期化
D3D_FEATURE_LEVEL featureLevel;
for (auto lv : levels) {
	if (D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK)
	{
		featureLevel = lv;
		break;//生成可能なバージョンが見つかったらループを打ち切り
	}
}

result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
	IID_PPV_ARGS(&_cmdAllocator));
result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
	_cmdAllocator, nullptr,
	IID_PPV_ARGS(&_cmdList));

D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
// タイムアウトなし
cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
// アダプターを1つしか使わないときは0でよい 
cmdQueueDesc.NodeMask = 0;
// プライオリティは特に指定なし
cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
// コマンドリストと合わせる
cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
// キュー生成
result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
swapchainDesc.Width = window_width;
swapchainDesc.Height = window_height;
swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
swapchainDesc.Stereo = false;
swapchainDesc.SampleDesc.Count = 1;
swapchainDesc.SampleDesc.Quality = 0;
swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
swapchainDesc.BufferCount = 2;
swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
result = _dxgiFactory->CreateSwapChainForHwnd(
	_cmdQueue, hwnd,
	&swapchainDesc, nullptr, nullptr,
	(IDXGISwapChain1**)&_swapchain);

// ウィンドウ表示 
ShowWindow(hwnd, SW_SHOW);

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};

Vertex vertices[] = {
	{{-0.4f,-0.7f,0.0f},{0.0f,1.0f}},
	{{-0.4f,+0.7f,0.0f},{0.0f,0.0f}},
	{ {+0.4f,-0.7f,0.0f},{1.0f,1.0f}},
	{{+0.4f,+0.7f,0.0f},{1.0f,0.0f}},
};

unsigned short indices[] = {
	0,1,2,
	2,1,3
};

D3D12_HEAP_PROPERTIES heapprop = {};
heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

D3D12_RESOURCE_DESC resdesc = {};
resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
resdesc.Width = sizeof(vertices); // 頂点情報が入るだけのサイズ
resdesc.Height = 1;
resdesc.DepthOrArraySize = 1;
resdesc.MipLevels = 1;
resdesc.Format = DXGI_FORMAT_UNKNOWN;
resdesc.SampleDesc.Count = 1;
resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

ID3D12Resource* vertBuff = nullptr;

result = _dev->CreateCommittedResource(
	&heapprop,
	D3D12_HEAP_FLAG_NONE,
	&resdesc,
	D3D12_RESOURCE_STATE_GENERIC_READ,
	nullptr,
	IID_PPV_ARGS(&vertBuff));

Vertex* vertMap = nullptr;
result = vertBuff->Map(0, nullptr, (void**)&vertMap);
std::copy(std::begin(vertices), std::end(vertices), vertMap);
vertBuff->Unmap(0, nullptr);

D3D12_VERTEX_BUFFER_VIEW vbView = {};
vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();//バッファーの仮想アドレス
vbView.SizeInBytes = sizeof(vertices);//全バイト数
vbView.StrideInBytes = sizeof(vertices[0]);//1頂点あたりのバイト数

D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//レンダーターゲットビュー
heapDesc.NodeMask = 0;
heapDesc.NumDescriptors = 2;//表裏の二つ
heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

ID3D12DescriptorHeap* rtvHeaps = nullptr;
result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

DXGI_SWAP_CHAIN_DESC swcDesc = {};
result = _swapchain->GetDesc(&swcDesc);

std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
{
	result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
	D3D12_CPU_DESCRIPTOR_HANDLE handle
		= rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
}

ID3D12Resource* idxBuff = nullptr;

result = _dev->CreateCommittedResource(
	&heapprop,
	D3D12_HEAP_FLAG_NONE,
	&resdesc,
	D3D12_RESOURCE_STATE_GENERIC_READ,
	nullptr,
	IID_PPV_ARGS(&idxBuff));

unsigned short* mappeIdx = nullptr;
idxBuff->Map(0, nullptr, (void**)&mappeIdx);
std::copy(std::begin(indices), std::end(indices), mappeIdx);
idxBuff->Unmap(0, nullptr);

D3D12_INDEX_BUFFER_VIEW ibView = {};
ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
ibView.Format = DXGI_FORMAT_R16_UINT;
ibView.SizeInBytes = sizeof(indices);

ID3DBlob* _vsBlob = nullptr;
ID3DBlob* _psBlob = nullptr;

ID3DBlob* errorBlob = nullptr;
result = D3DCompileFromFile(
	L"BasicVertexShader.hlsl",//シェーダー名
	nullptr,
	D3D_COMPILE_STANDARD_FILE_INCLUDE,
	"BasicVS", "vs_5_0",
	D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
	0,
	&_vsBlob, &errorBlob);
if (FAILED(result)) {
	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
		::OutputDebugStringA("ファイルが見当たりません");
	}
	else {
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(), errstr.begin());
		errstr += "\n";
		OutputDebugStringA(errstr.c_str());
	}
	exit(1);
}

result = D3DCompileFromFile(
	L"BasicPixelShader.hlsl",//シェーダー名
	nullptr,
	D3D_COMPILE_STANDARD_FILE_INCLUDE,
	"BasicPS", "ps_5_0",
	D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
	0,
	&_psBlob, &errorBlob);
if (FAILED(result)) {
	if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
		::OutputDebugStringA("ファイルが見当たりません");
	}
	else {
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());
		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(), errstr.begin());
		errstr += "\n";
		OutputDebugStringA(errstr.c_str());
	}
	exit(1);
}

D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
	{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
	D3D12_APPEND_ALIGNED_ELEMENT,
	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
	},
	{//uv
		"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,
		0,D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
	},
};



D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

gpipeline.pRootSignature = nullptr;

gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();
gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
gpipeline.RasterizerState.DepthClipEnable = true;

gpipeline.BlendState.AlphaToCoverageEnable = false;
gpipeline.BlendState.IndependentBlendEnable = false;

D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlandDesc = {};

renderTargetBlandDesc.BlendEnable = false;

renderTargetBlandDesc.LogicOpEnable = false;
renderTargetBlandDesc.RenderTargetWriteMask =
D3D12_COLOR_WRITE_ENABLE_ALL;

gpipeline.BlendState.RenderTarget[0] = renderTargetBlandDesc;
gpipeline.InputLayout.pInputElementDescs = inputLayout;//レイアウト戦闘アドレス
gpipeline.InputLayout.NumElements = _countof(inputLayout);//英アウト配列数
gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

gpipeline.NumRenderTargets = 1;
gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

gpipeline.SampleDesc.Count = 1;
gpipeline.SampleDesc.Quality = 0;

D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

ID3D10Blob* rootSigBlob = nullptr;
result = D3D12SerializeRootSignature(
	&rootSignatureDesc,
	D3D_ROOT_SIGNATURE_VERSION_1_0,
	&rootSigBlob,
	&errorBlob);

ID3D12RootSignature* rootsignature = nullptr;
result = _dev->CreateRootSignature(
	0,
	rootSigBlob->GetBufferPointer(),
	rootSigBlob->GetBufferSize(),
	IID_PPV_ARGS(&rootsignature));
rootSigBlob->Release();

gpipeline.pRootSignature = rootsignature;

ID3D12PipelineState* _pipelinestate = nullptr;
result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));

D3D12_VIEWPORT viewport = {};
viewport.Width = window_width;
viewport.Height = window_height;
viewport.TopLeftX = 0;
viewport.TopLeftY = 0;
viewport.MaxDepth = 1.0f;
viewport.MinDepth = 0.0f;

D3D12_RECT scissorrect = {};
scissorrect.top = 0;
scissorrect.left = 0;
scissorrect.right = scissorrect.left + window_width;
scissorrect.bottom = scissorrect.top + window_height;

float clrR = 0.0;
float clrG = 0.3;
float clrB = 0.6;


struct  TexRGBA
{
	unsigned char R, G, B, A;
};

std::vector<TexRGBA> texturedata(256 * 256);
for (auto& rgba : texturedata) {
	rgba.R = rand() % 256;
	rgba.G = rand() % 256;
	rgba.B = rand() % 256;
	rgba.A = 255;
}

D3D12_HEAP_PROPERTIES texHeapProp = {};
texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
texHeapProp.CreationNodeMask = 0;
texHeapProp.VisibleNodeMask = 0;

D3D12_RESOURCE_DESC resDesc = {};
resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
resDesc.Width = 256;
resDesc.Height = 256;
resDesc.DepthOrArraySize = 1;
resDesc.SampleDesc.Count = 1;
resDesc.SampleDesc.Quality = 0;
resDesc.MipLevels = 1;
resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

ID3D12Resource* texbuff = nullptr;
result = _dev->CreateCommittedResource(
	&texHeapProp,
	D3D12_HEAP_FLAG_NONE,
	&resDesc,
	D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	nullptr,
	IID_PPV_ARGS(&texbuff));

result = texbuff->WriteToSubresource(
	0,
	nullptr,
	texturedata.data(),
	sizeof(TexRGBA) * 256,
	sizeof(TexRGBA) * (UINT)texturedata.size()
);

ID3D12DescriptorHeap* texDescHeap = nullptr;
D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
descHeapDesc.NodeMask = 0;
descHeapDesc.NumDescriptors = 1;
descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
result = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&texDescHeap));

D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
srvDesc.Shader4ComponentMapping =
D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
srvDesc.Texture2D.MipLevels = 1;

_dev->CreateShaderResourceView(
	texbuff,
	&srvDesc,
	texDescHeap->GetCPUDescriptorHandleForHeapStart()
);

while (true)
{
	MSG msg;
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		// アプリケーションが終わるときに message が WM_QUIT になる 
		if (msg.message == WM_QUIT)
			break;
		TranslateMessage(&msg); DispatchMessage(&msg);
	}
	//DirectX処理
	//バックバッファのインデックスを取得
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

	_cmdList->SetPipelineState(_pipelinestate);

	//D3D12_RESOURCE_BARRIER BarrierDesc = {};
	//BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;// 
	//BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;//
	//BarrierDesc.Transition.pResource = _backBuffers[bbIdx];////
	//BarrierDesc.Transition.Subresource = 0;
	//BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;//PRESENT
	//BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;//RT 
	//_cmdList->ResourceBarrier(1, &BarrierDesc);//

		// レンダーターゲットを指定
	auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);


	// 画面クリア
	clrR = clrR + 0.01;
	clrG = clrG + 0.01;
	clrB = clrB + 0.01;
	if (clrR > 1)clrR -= 1;
	if (clrG > 1)clrG -= 1;
	if (clrB > 1)clrB -= 1;
	float clearColor[] = { clrR,clrG,clrB,1.0 };

	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	//// 前後だけ入れ替える
	//BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//_cmdList->ResourceBarrier(1, &BarrierDesc);

	_cmdList->SetGraphicsRootSignature(rootsignature);

	_cmdList->RSSetViewports(1, &viewport);
	_cmdList->RSSetScissorRects(1, &scissorrect);

	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_cmdList->IASetVertexBuffers(0, 1, &vbView);
	_cmdList->IASetIndexBuffer(&ibView);

	_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);

	// 命令のクローズ 
	_cmdList->Close();
	// コマンドリストの実行
	ID3D12CommandList* cmdlists[] = { _cmdList };
	_cmdQueue->ExecuteCommandLists(1, cmdlists);
	_cmdAllocator->Reset(); // キューをクリア
	_cmdList->Reset(_cmdAllocator, nullptr); // 再びコマンドリストを貯める処理
	// フリップ
	_swapchain->Present(1, 0);

}
// もうクラスは使わないので登録解除する 
UnregisterClass(w.lpszClassName, w.hInstance);
return 0;
}