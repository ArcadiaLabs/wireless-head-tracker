#pragma once

#define ThrowD3DException(rslt, msg)		ThrowD3DExceptionFileLine(rslt, msg, WIDEN(__FILE__), __LINE__)

//
// the direct3D object
//

class Direct3D
{
private:
	IDirect3D9*		_pD3D;

public:
	Direct3D();

	~Direct3D()
	{
		// shutdown Direct3D
		_pD3D->Release();
	}

	std::string GetAdapterName() const;
	void GetAdapterDisplayMode(D3DDISPLAYMODE& d3ddm);
	bool CheckDeviceMultiSampleType(D3DFORMAT format);
	IDirect3DDevice9* CreateDevice(HWND hwnd, D3DPRESENT_PARAMETERS& d3d_pp);
};

//
// device
//

class DeviceD3D
{
	friend class Object3D;
private:
	IDirect3DDevice9*		_pDevice;

	D3DPRESENT_PARAMETERS	_d3d_pp;
	
	bool					_lighting_enabled;

public:
	DeviceD3D()
		: _pDevice(0)
	{}

	~DeviceD3D();

	void Release()
	{
		if (_pDevice != 0)
			_pDevice->Release();

		_pDevice = 0;
	}
	
	void Init(Direct3D& d3d, HWND d3d_win);
	void EnableLight();
	void DisableLight();
	void SetView(const D3DXVECTOR3& camera_pos, const D3DXVECTOR3& look_at, const D3DXVECTOR3& up);
	bool IsValid();
	void Clear();

	void BeginScene();
	void EndScene();
	void Present();

	void SetProjectionTransform(D3DXMATRIX &matProjection);
	void GetViewport(D3DVIEWPORT9& viewport);
	void SetViewport(D3DVIEWPORT9& viewport);

	void SetCulling(const size_t cull_mode);

	IDirect3DSurface9* GetBackBuffer();
	IDirect3DVertexBuffer9* CreateVertexBuffer(const int vcount);
};

//
// the vertex buffer -- every CD3DObject has one embedded
//

class CVertexBuffer
{
	friend class Object3D;
private:
	IDirect3DVertexBuffer9*		_pvb;

	size_t		_vertex_count;

public:
	CVertexBuffer();
	CVertexBuffer(const CVertexBuffer& c);

	~CVertexBuffer()
	{
		Release();
	}

	bool IsEmpty() const
	{
		return _pvb == 0;
	}

	void Alloc(DeviceD3D& dev, const int vcount);
	char* Lock();
	void Unlock();

	void Release();
};

//
// the only vertex format we'll be using
//

struct CSimpleVertex
{
	enum {fvf_id = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_NORMAL};

	D3DXVECTOR3	pos;
	D3DXVECTOR3	normal;
	D3DCOLOR	diffuse;

	// used to transform the vertex with STL (for_each)
	struct transform_t
	{
		D3DXMATRIX		transform_matrix;
		D3DXVECTOR4		out;

		transform_t(const D3DXMATRIX tm) : transform_matrix(tm)		{}

		void operator () (CSimpleVertex& v)
		{
			D3DXVec3Transform(&out, &v.pos, &transform_matrix);

			v.pos.x = out.x;
			v.pos.y = out.y;
			v.pos.z = out.z;
		}
	};
};


//
// base class for the objects we'll have in the scene
//

class Object3D
{
protected:
	// this holds the vertices for the object
	std::vector<CSimpleVertex>	_vertices;

	// this is the vertex buffer for the object
	CVertexBuffer				_vertex_buffer;

	void MakeVertexBuffer(DeviceD3D& dev);

public:

	virtual void Render(DeviceD3D& dev);

	void Release()
	{
		_vertex_buffer.Release();
	}
};


//
// the camera class
//

class CCamera
{
private:
	D3DXVECTOR3		_up;
	D3DXVECTOR3		_camera_pos;

	float			_rotY;		// the current camera rotation and scale
	float			_rotX;
	float			_scale;

	DeviceD3D&		_dev;

	void CalcCamera();

public:
	CCamera(DeviceD3D& d);

	// sets the view transformation on the device
	void RefreshPos();

	void SetRotation(float deltaY, float deltaX);
	void Zoom(const int zoom);
};

// builds a cube of given dimensions around (0,0,0)
void BuildCube(std::vector<CSimpleVertex>& v, float Width, float Height, float Depth);

// builds a cube of given dimensions at given coordinates
void BuildCube(std::vector<CSimpleVertex>& v, float Width, float Height, float Depth, float x, float y, float z);

// returns the angle between 2 NORMALIZED vectors
inline float GetAngle(const D3DXVECTOR3& v1, const D3DXVECTOR3& v2)
{
	return acos(D3DXVec3Dot(&v1, &v2));
}