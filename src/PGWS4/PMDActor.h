﻿#pragma once
#include<d3d12.h>
#include<DirectXMath.h>
#include<vector>
#include<map>
#include<string>
#include<wrl.h>

class Dx12Wrapper;
class PMDRenderer;

class PMDActor
{
	friend PMDRenderer;
private:
	PMDRenderer& _renderer;
	Dx12Wrapper& _dx12;

	// 描画状態の設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _materialDescHeap = nullptr;
//	Microsoft::WRL::ComPtr<ID3D12Resource> _transformMat = nullptr;//座標変換行列(今はワールドのみ)
//	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _transformHeap = nullptr;//座標変換ヒープ

	// マテリアルデータ
	struct MaterialForHlsl// シェーダー側に投げられるマテリアルデータ
	{
		DirectX::XMFLOAT3 diffuse; // ディフューズ色
		float alpha; // ディフューズα
		DirectX::XMFLOAT3 specular; // スペキュラ色
		float specularity; // スペキュラの強さ（乗算値）
		DirectX::XMFLOAT3 ambient; // アンビエント色
	};

	struct AdditionalMaterial// それ以外のマテリアルデータ
	{
		std::string texPath; // テクスチャファイルパス
		int toonIdx; // トゥーン番号
		bool edgeFlg; // マテリアルごとの輪郭線フラグ
	};

	struct Material// 全体をまとめるデータ
	{
		unsigned int indicesNum; // インデックス数
		MaterialForHlsl material;
		AdditionalMaterial additional;
	};
	unsigned int _materialNum; // マテリアル数
	std::vector<Material> materials;
	Microsoft::WRL::ComPtr<ID3D12Resource> _materialBuff = nullptr;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _textureResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _sphResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _spaResources;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _toonResources;
	Microsoft::WRL::ComPtr< ID3D12DescriptorHeap> _materialHeap = nullptr;//マテリアルヒープ(5個ぶん)


//	struct Transform {
		//内部に持ってるXMMATRIXメンバが16バイトアライメントであるため
		//Transformをnewする際には16バイト境界に確保する
//		void* operator new(size_t size);
//		DirectX::XMMATRIX world;
//	};
//	Transform _transform;
//	Transform* _mappedTransform = nullptr;
//	Microsoft::WRL::ComPtr<ID3D12Resource> _transformBuff = nullptr;


	//頂点
	Microsoft::WRL::ComPtr<ID3D12Resource> _vertBuff = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> _idxBuff = nullptr;
	D3D12_VERTEX_BUFFER_VIEW _vbView = {};
	D3D12_INDEX_BUFFER_VIEW _ibView = {};

	HRESULT LoadPMDFile(const char* path);//PMDファイルのロード
	D3D12_CONSTANT_BUFFER_VIEW_DESC CreateMaterialData();//読み込んだマテリアルをもとにマテリアルバッファを作成
	void CreateMaterialAndTextureView(D3D12_CONSTANT_BUFFER_VIEW_DESC& matCBVDesc);//マテリアル＆テクスチャのビューを作成


	float _angle;//テスト用Y軸回転
public:
	PMDActor(const char* filepath, PMDRenderer& renderer);
	~PMDActor();

	///クローンは頂点およびマテリアルは共通のバッファを見るようにする
	PMDActor* Clone();

	void Update();
	void Draw();
};

