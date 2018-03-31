//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment (lib, "D3D10_1.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "D2D1.lib")
#pragma comment (lib, "dwrite.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>
#include <D3D10_1.h>
#include <DXGI.h>
#include <D2D1.h>
#include <sstream>
#include <dwrite.h>
#include <dinput.h>
#include <vector>
#include <fstream>
#include <istream>

//Global Declarations - Interfaces//
IDXGISwapChain* SwapChain;
ID3D11Device* d3d11Device;
ID3D11DeviceContext* d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;
ID3D11DepthStencilView* depthStencilView;
ID3D11Texture2D* depthStencilBuffer;
ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* VS_Buffer;
ID3D10Blob* PS_Buffer;
ID3D11InputLayout* vertLayout;
ID3D11Buffer* cbPerObjectBuffer;
ID3D11RasterizerState* CCWcullMode;
ID3D11RasterizerState* CWcullMode;
ID3D11SamplerState* LinearSamplerState;
ID3D11Buffer* cbPerFrameBuffer;
ID3D11Texture2D *BackBuffer11;
IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;
ID3D11RasterizerState* RSCullNone;
ID3D11BlendState* Transparency;

//Global Declarations - Others//
LPCTSTR WndClassName = L"firstwindow";
HWND hwnd = NULL;
HRESULT hr;

int Width  = 800;
int Height = 600;

DIMOUSESTATE mouseLastState;
LPDIRECTINPUT8 DirectInput;

XMMATRIX WVP;
XMMATRIX camView;
XMMATRIX camProjection;
XMMATRIX camRotationMatrix;

XMVECTOR camPosition;
XMVECTOR camTarget;
XMVECTOR camUp;
XMVECTOR DefaultForward = XMVectorSet(0.0f,0.0f,1.0f, 0.0f);
XMVECTOR DefaultRight = XMVectorSet(1.0f,0.0f,0.0f, 0.0f);
XMVECTOR camForward = XMVectorSet(0.0f,0.0f,1.0f, 0.0f);
XMVECTOR camRight = XMVectorSet(1.0f,0.0f,0.0f, 0.0f);

float moveLeftRight = 0.0f;
float moveBackForward = 0.0f;
float camYaw = 0.0f;
float camPitch = 0.0f;

XMMATRIX Rotation;
XMMATRIX Scale;
XMMATRIX Translation;
XMMATRIX Rotationx;
XMMATRIX Rotationz;
XMMATRIX Rotationy;

double countsPerSecond = 0.0;
__int64 CounterStart = 0;
int frameCount = 0;
int fps = 0;
__int64 frameTimeOld = 0;
double frameTime;

//Function Prototypes//
bool InitializeDirect3d11App(HINSTANCE hInstance);
void CleanUp();
bool InitScene();
void DrawScene();
void UpdateScene(double time);

void UpdateCamera();

void StartTimer();
double GetTime();
double GetFrameTime();

bool InitializeWindow(HINSTANCE hInstance,
    int ShowWnd,
    int width, int height,
    bool windowed);
int messageloop();

bool InitDirectInput(HINSTANCE hInstance);
void DetectInput(double time);

LRESULT CALLBACK WndProc(HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);

///////////////**************new**************////////////////////
// Texture manager structure, So all the textures are nice and grouped together
struct TextureManager
{
    std::vector<ID3D11ShaderResourceView*> TextureList;
    std::vector<std::wstring> TextureNameArray;     // So we don't load in the same texture twice
};

// Create material structure
struct SurfaceMaterial
{
    std::wstring MatName;   // So we can match the subset with it's material

    // Surface's colors
    XMFLOAT4 Diffuse;       // Transparency (Alpha) stored in 4th component
    XMFLOAT3 Ambient;
    XMFLOAT4 Specular;      // Specular power stored in 4th component

    // Texture ID's to look up texture in SRV array
    int DiffuseTextureID;
    int AmbientTextureID;
    int SpecularTextureID;
    int AlphaTextureID;
    int NormMapTextureID;

    // Booleans so we don't implement techniques we don't need
    bool HasDiffTexture;
    bool HasAmbientTexture;
    bool HasSpecularTexture;
    bool HasAlphaTexture;
    bool HasNormMap;
    bool IsTransparent;
};

// Model Structure
struct ObjModel
{
    int Subsets;                        // Number of subsets in obj model
    ID3D11Buffer* VertBuff;             // Models vertex buffer
    ID3D11Buffer* IndexBuff;            // Models index buffer
    std::vector<XMFLOAT3> Vertices;     // Models vertex positions list
    std::vector<DWORD> Indices;         // Models index list
    std::vector<int> SubsetIndexStart;  // Subset's index offset
    std::vector<int> SubsetMaterialID;  // Lookup ID for subsets surface material
    XMMATRIX World;                     // Models world matrix
	std::vector<XMFLOAT3> AABB;			// Stores models AABB (min vertex, max vertex, and center)
										// Where AABB[0] is the min Vertex, and AABB[1] is the max vertex
	XMFLOAT3 Center;					// True center of the model
	float BoundingSphere;				// Model's bounding sphere
};

TextureManager textureMgr;
std::vector<SurfaceMaterial> material;
ObjModel groundModel;

//Define LoadObjModel function after we create surfaceMaterial structure
bool LoadObjModel(ID3D11Device* device,     // Pointer to the device
    std::wstring Filename,                      // obj model filename (model.obj)
    ObjModel& Model,                            // Pointer to an objModel object
    std::vector<SurfaceMaterial>& MatArray,     // Array of material structures
    TextureManager& TexMgr,                     // Holds games textures
    bool IsRHCoordSys,                          // True if model was created in right hand coord system
    bool ComputeNormals);                       // True to compute the normals, false to use the files normals
///////////////**************new**************////////////////////

struct Light
{
    Light()
    {
        ZeroMemory(this, sizeof(Light));
    }
    XMFLOAT3 pos;
    float range;
    XMFLOAT3 dir;
    float cone;
    XMFLOAT3 att;
    float pad2;
    XMFLOAT4 ambient;
    XMFLOAT4 diffuse;
};

Light light;

// Create effects constant buffer's structure //
struct cbPerObject
{
    XMMATRIX  WVP;
    XMMATRIX World;

    // These will be used for the pixel shader
    XMFLOAT4 difColor;
    // Because of HLSL structure packing, we will use windows BOOL
    // instead of bool because HLSL packs things into 4 bytes, and
    // bool is only one byte, where BOOL is 4 bytes
    BOOL hasTexture;
    BOOL hasNormMap;
};

cbPerObject cbPerObj;

struct cbPerFrame
{
    Light  light;
};

cbPerFrame constbuffPerFrame;

struct Vertex
{
    XMFLOAT3 pos;
    XMFLOAT2 texCoord;
    XMFLOAT3 normal;
    XMFLOAT3 tangent;
    XMFLOAT3 biTangent;
};

UINT stride = sizeof( Vertex );
UINT offset = 0;

D3D11_INPUT_ELEMENT_DESC layout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
    { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
    { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
};
UINT numElements = ARRAYSIZE(layout);

int WINAPI WinMain(HINSTANCE hInstance, //Main windows function
    HINSTANCE hPrevInstance, 
    LPSTR lpCmdLine,
    int nShowCmd)
{

    if(!InitializeWindow(hInstance, nShowCmd, Width, Height, true))
    {
        MessageBox(0, L"Window Initialization - Failed",
            L"Error", MB_OK);
        return 0;
    }

    if(!InitializeDirect3d11App(hInstance)) //Initialize Direct3D
    {
        MessageBox(0, L"Direct3D Initialization - Failed",
            L"Error", MB_OK);
        return 0;
    }

    if(!InitScene())    //Initialize our scene
    {
        MessageBox(0, L"Scene Initialization - Failed",
            L"Error", MB_OK);
        return 0;
    }

    if(!InitDirectInput(hInstance))
    {
        MessageBox(0, L"Direct Input Initialization - Failed",
            L"Error", MB_OK);
        return 0;
    }

    messageloop();

    CleanUp();    

    return 0;
}

bool InitializeWindow(HINSTANCE hInstance,
    int ShowWnd,
    int width, int height,
    bool windowed)
{
    typedef struct _WNDCLASS {
        UINT cbSize;
        UINT style;
        WNDPROC lpfnWndProc;
        int cbClsExtra;
        int cbWndExtra;
        HANDLE hInstance;
        HICON hIcon;
        HCURSOR hCursor;
        HBRUSH hbrBackground;
        LPCTSTR lpszMenuName;
        LPCTSTR lpszClassName;
    } WNDCLASS;

    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WndClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, L"Error registering class",  
            L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    hwnd = CreateWindowEx(
        NULL,
        WndClassName,
        L"Ojb Model Loader",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        NULL,
        NULL,
        hInstance,
        NULL
        );

    if (!hwnd)
    {
        MessageBox(NULL, L"Error creating window",
            L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, ShowWnd);
    UpdateWindow(hwnd);

    return true;
}

bool InitializeDirect3d11App(HINSTANCE hInstance)
{   
    // Describe our SwapChain Buffer
    DXGI_MODE_DESC bufferDesc;

    ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

    bufferDesc.Width = Width;
    bufferDesc.Height = Height;
    bufferDesc.RefreshRate.Numerator = 60;
    bufferDesc.RefreshRate.Denominator = 1;
    bufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Describe our SwapChain
    DXGI_SWAP_CHAIN_DESC swapChainDesc; 

    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    swapChainDesc.BufferDesc = bufferDesc;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = hwnd; 
    swapChainDesc.Windowed = true; 
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    // Create our Direct3D 11 Device and SwapChain
    hr = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        NULL, NULL, D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);
    
    // Create our BackBuffer and Render Target
    hr = SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&BackBuffer11 );
    hr = d3d11Device->CreateRenderTargetView( BackBuffer11, NULL, &renderTargetView );

    // Describe our Depth/Stencil Buffer
    D3D11_TEXTURE2D_DESC depthStencilDesc;

    depthStencilDesc.Width     = Width;
    depthStencilDesc.Height    = Height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count   = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0; 
    depthStencilDesc.MiscFlags      = 0;

    // Create the Depth/Stencil View
    d3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
    d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

    return true;
}

bool InitDirectInput(HINSTANCE hInstance)
{
    hr = DirectInput8Create(hInstance,
        DIRECTINPUT_VERSION,
        IID_IDirectInput8,
        (void**)&DirectInput,
        NULL); 

    hr = DirectInput->CreateDevice(GUID_SysKeyboard,
        &DIKeyboard,
        NULL);

    hr = DirectInput->CreateDevice(GUID_SysMouse,
        &DIMouse,
        NULL);

    hr = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
    hr = DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

    hr = DIMouse->SetDataFormat(&c_dfDIMouse);
    hr = DIMouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

    return true;
}

void UpdateCamera()
{
    camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);
    camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix );
    camTarget = XMVector3Normalize(camTarget);

    XMMATRIX RotateYTempMatrix;
    RotateYTempMatrix = XMMatrixRotationY(camYaw);

    //walk
    camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
    camUp = XMVector3TransformCoord(camUp, RotateYTempMatrix);
    camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);

    camPosition += moveLeftRight*camRight;
    camPosition += moveBackForward*camForward;

    moveLeftRight = 0.0f;
    moveBackForward = 0.0f;

    camTarget = camPosition + camTarget;    

    camView = XMMatrixLookAtLH( camPosition, camTarget, camUp );
}

void DetectInput(double time)
{
    DIMOUSESTATE mouseCurrState;

    BYTE keyboardState[256];

    DIKeyboard->Acquire();
    DIMouse->Acquire();

    DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);

    DIKeyboard->GetDeviceState(sizeof(keyboardState),(LPVOID)&keyboardState);

    if(keyboardState[DIK_ESCAPE] & 0x80)
        PostMessage(hwnd, WM_DESTROY, 0, 0);

    float speed = 10.0f * time;

    if(keyboardState[DIK_A] & 0x80)
    {
        moveLeftRight -= speed;
    }
    if(keyboardState[DIK_D] & 0x80)
    {
        moveLeftRight += speed;
    }
    if(keyboardState[DIK_W] & 0x80)
    {
        moveBackForward += speed;
    }
    if(keyboardState[DIK_S] & 0x80)
    {
        moveBackForward -= speed;
    }
    if((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
    {
        camYaw += mouseLastState.lX * 0.001f;

        camPitch += mouseCurrState.lY * 0.001f;

        mouseLastState = mouseCurrState;
    }

    UpdateCamera();

    return;
}


void CleanUp()
{
    SwapChain->SetFullscreenState(false, NULL);
    PostMessage(hwnd, WM_DESTROY, 0, 0);

    //Release the COM Objects we created
    SwapChain->Release();
    d3d11Device->Release();
    d3d11DevCon->Release();
    renderTargetView->Release();
    VS->Release();
    PS->Release();
    VS_Buffer->Release();
    PS_Buffer->Release();
    vertLayout->Release();
    depthStencilView->Release();
    depthStencilBuffer->Release();
    cbPerObjectBuffer->Release();
    CCWcullMode->Release();
    CWcullMode->Release();
    RSCullNone->Release();
    Transparency->Release();

    cbPerFrameBuffer->Release();

    DIKeyboard->Unacquire();
    DIMouse->Unacquire();
    DirectInput->Release();

}

///////////////**************new**************////////////////////
bool LoadObjModel(ID3D11Device* device,
    std::wstring Filename, 
    ObjModel& Model,
    std::vector<SurfaceMaterial>& material, 
    TextureManager& TexMgr,
    bool IsRHCoordSys,
    bool ComputeNormals)
{
    HRESULT hr = 0;

    std::wifstream fileIn (Filename.c_str());   // Open file
    std::wstring meshMatLib;                    // String to hold our obj material library filename (model.mtl)

    // Arrays to store our model's information
    std::vector<DWORD> indices;
    std::vector<XMFLOAT3> vertPos;
    std::vector<XMFLOAT3> vertNorm;
    std::vector<XMFLOAT2> vertTexCoord;
    std::vector<std::wstring> meshMaterials;

    // Vertex definition indices
    std::vector<int> vertPosIndex;
    std::vector<int> vertNormIndex;
    std::vector<int> vertTCIndex;

    // Make sure we have a default if no tex coords or normals are defined
    bool hasTexCoord = false;
    bool hasNorm = false;

    // Temp variables to store into vectors
    std::wstring meshMaterialsTemp;
    int vertPosIndexTemp;
    int vertNormIndexTemp;
    int vertTCIndexTemp;

    wchar_t checkChar;      // The variable we will use to store one char from file at a time
    std::wstring face;      // Holds the string containing our face vertices
    int vIndex = 0;         // Keep track of our vertex index count
    int triangleCount = 0;  // Total Triangles
    int totalVerts = 0;
    int meshTriangles = 0;

    //Check to see if the file was opened
    if (fileIn)
    {
        while(fileIn)
        {           
            checkChar = fileIn.get();   //Get next char

            switch (checkChar)
            {       
                // A comment. Skip rest of the line
            case '#':
                checkChar = fileIn.get();
                while(checkChar != '\n')
                    checkChar = fileIn.get();
                break;

                    // Get Vertex Descriptions;
            case 'v':
                checkChar = fileIn.get();
                if(checkChar == ' ')  // v - vert position
                {
                    float vz, vy, vx;
                    fileIn >> vx >> vy >> vz;   // Store the next three types

                    if(IsRHCoordSys)            // If model is from an RH Coord System
                        vertPos.push_back(XMFLOAT3( vx, vy, vz * -1.0f));   // Invert the Z axis
                    else
                        vertPos.push_back(XMFLOAT3( vx, vy, vz));
                }
                if(checkChar == 't')  // vt - vert tex coords
                {           
                    float vtcu, vtcv;
                    fileIn >> vtcu >> vtcv;     // Store next two types

                    if(IsRHCoordSys)            // If model is from an RH Coord System
                        vertTexCoord.push_back(XMFLOAT2(vtcu, 1.0f-vtcv));  // Reverse the "v" axis
                    else
                        vertTexCoord.push_back(XMFLOAT2(vtcu, vtcv));   

                    hasTexCoord = true;         // We know the model uses texture coords
                }
                if(checkChar == 'n')          // vn - vert normal
                {
                    float vnx, vny, vnz;
                    fileIn >> vnx >> vny >> vnz;    // Store next three types

                    if(IsRHCoordSys)                // If model is from an RH Coord System
                        vertNorm.push_back(XMFLOAT3( vnx, vny, vnz * -1.0f ));  // Invert the Z axis
                    else
                        vertNorm.push_back(XMFLOAT3( vnx, vny, vnz ));  

                    hasNorm = true;                 // We know the model defines normals
                }
                break;

                // New group (Subset)
            case 'g': // g - defines a group
                checkChar = fileIn.get();
                if(checkChar == ' ')
                {
                    Model.SubsetIndexStart.push_back(vIndex);       // Start index for this subset
                    Model.Subsets++;
                }
                break;

                // Get Face Index
            case 'f': // f - defines the faces
                checkChar = fileIn.get();
                if(checkChar == ' ')
                {
                    face = L"";               // Holds all vertex definitions for the face (eg. "1\1\1 2\2\2 3\3\3")
                    std::wstring VertDef;   // Holds one vertex definition at a time (eg. "1\1\1")
                    triangleCount = 0;      // Keep track of the triangles for this face, since we will have to 
                                            // "retriangulate" faces with more than 3 sides

                    checkChar = fileIn.get();
                    while(checkChar != '\n')
                    {
                        face += checkChar;          // Add the char to our face string
                        checkChar = fileIn.get();   // Get the next Character
                        if(checkChar == ' ')      // If its a space...
                            triangleCount++;        // Increase our triangle count
                    }

                    // Check for space at the end of our face string
                    if(face[face.length()-1] == ' ')
                        triangleCount--;    // Each space adds to our triangle count

                    triangleCount -= 1;     // Ever vertex in the face AFTER the first two are new faces

                    std::wstringstream ss(face);

                    if(face.length() > 0)
                    {
                        int firstVIndex, lastVIndex;    // Holds the first and last vertice's index

                        for(int i = 0; i < 3; ++i)      // First three vertices (first triangle)
                        {
                            ss >> VertDef;  // Get vertex definition (vPos/vTexCoord/vNorm)

                            std::wstring vertPart;
                            int whichPart = 0;      // (vPos, vTexCoord, or vNorm)

                            // Parse this string
                            for(int j = 0; j < VertDef.length(); ++j)
                            {
                                if(VertDef[j] != '/') // If there is no divider "/", add a char to our vertPart
                                    vertPart += VertDef[j];

                                // If the current char is a divider "/", or its the last character in the string
                                if(VertDef[j] == '/' || j ==  VertDef.length()-1)
                                {
                                    std::wistringstream wstringToInt(vertPart); // Used to convert wstring to int

                                    if(whichPart == 0)  // If it's the vPos
                                    {
                                        wstringToInt >> vertPosIndexTemp;
                                        vertPosIndexTemp -= 1;      // subtract one since c++ arrays start with 0, and obj start with 1

                                        // Check to see if the vert pos was the only thing specified
                                        if(j == VertDef.length()-1)
                                        {
                                            vertNormIndexTemp = 0;
                                            vertTCIndexTemp = 0;
                                        }
                                    }

                                    else if(whichPart == 1) // If vTexCoord
                                    {
                                        if(vertPart != L"")   // Check to see if there even is a tex coord
                                        {
                                            wstringToInt >> vertTCIndexTemp;
                                            vertTCIndexTemp -= 1;   // subtract one since c++ arrays start with 0, and obj start with 1
                                        }
                                        else    // If there is no tex coord, make a default
                                            vertTCIndexTemp = 0;

                                        // If the cur. char is the second to last in the string, then
                                        // there must be no normal, so set a default normal
                                        if(j == VertDef.length()-1)
                                            vertNormIndexTemp = 0;

                                    }                               
                                    else if(whichPart == 2) // If vNorm
                                    {
                                        std::wistringstream wstringToInt(vertPart);

                                        wstringToInt >> vertNormIndexTemp;
                                        vertNormIndexTemp -= 1;     // subtract one since c++ arrays start with 0, and obj start with 1
                                    }

                                    vertPart = L"";   // Get ready for next vertex part
                                    whichPart++;    // Move on to next vertex part                  
                                }
                            }

                            // Check to make sure there is at least one subset
                            if(Model.Subsets == 0)
                            {
                                Model.SubsetIndexStart.push_back(vIndex);       // Start index for this subset
                                Model.Subsets++;
                            }

                            // Avoid duplicate vertices
                            bool vertAlreadyExists = false;
                            if(totalVerts >= 3) // Make sure we at least have one triangle to check
                            {
                                // Loop through all the vertices
                                for(int iCheck = 0; iCheck < totalVerts; ++iCheck)
                                {
                                    // If the vertex position and texture coordinate in memory are the same
                                    // As the vertex position and texture coordinate we just now got out
                                    // of the obj file, we will set this faces vertex index to the vertex's
                                    // index value in memory. This makes sure we don't create duplicate vertices
                                    if(vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists)
                                    {
                                        if(vertTCIndexTemp == vertTCIndex[iCheck])
                                        {
                                            Model.Indices.push_back(iCheck);        // Set index for this vertex
                                            vertAlreadyExists = true;       // If we've made it here, the vertex already exists
                                        }
                                    }
                                }
                            }

                            // If this vertex is not already in our vertex arrays, put it there
                            if(!vertAlreadyExists)
                            {
                                vertPosIndex.push_back(vertPosIndexTemp);
                                vertTCIndex.push_back(vertTCIndexTemp);
                                vertNormIndex.push_back(vertNormIndexTemp);
                                totalVerts++;   // We created a new vertex
                                Model.Indices.push_back(totalVerts-1);  // Set index for this vertex
                            }                           

                            // If this is the very first vertex in the face, we need to
                            // make sure the rest of the triangles use this vertex
                            if(i == 0)
                            {
                                firstVIndex = Model.Indices[vIndex];    //The first vertex index of this FACE

                            }

                            // If this was the last vertex in the first triangle, we will make sure
                            // the next triangle uses this one (eg. tri1(1,2,3) tri2(1,3,4) tri3(1,4,5))
                            if(i == 2)
                            {                               
                                lastVIndex = Model.Indices[vIndex]; // The last vertex index of this TRIANGLE
                            }
                            vIndex++;   // Increment index count
                        }

                        meshTriangles++;    // One triangle down

                        // If there are more than three vertices in the face definition, we need to make sure
                        // we convert the face to triangles. We created our first triangle above, now we will
                        // create a new triangle for every new vertex in the face, using the very first vertex
                        // of the face, and the last vertex from the triangle before the current triangle
                        for(int l = 0; l < triangleCount-1; ++l)    // Loop through the next vertices to create new triangles
                        {
                            // First vertex of this triangle (the very first vertex of the face too)
                            Model.Indices.push_back(firstVIndex);           // Set index for this vertex
                            vIndex++;

                            // Second Vertex of this triangle (the last vertex used in the tri before this one)
                            Model.Indices.push_back(lastVIndex);            // Set index for this vertex
                            vIndex++;

                            // Get the third vertex for this triangle
                            ss >> VertDef;

                            std::wstring vertPart;
                            int whichPart = 0;

                            // Parse this string (same as above)
                            for(int j = 0; j < VertDef.length(); ++j)
                            {
                                if(VertDef[j] != '/')
                                    vertPart += VertDef[j];
                                if(VertDef[j] == '/' || j ==  VertDef.length()-1)
                                {
                                    std::wistringstream wstringToInt(vertPart);

                                    if(whichPart == 0)
                                    {
                                        wstringToInt >> vertPosIndexTemp;
                                        vertPosIndexTemp -= 1;

                                        // Check to see if the vert pos was the only thing specified
                                        if(j == VertDef.length()-1)
                                        {
                                            vertTCIndexTemp = 0;
                                            vertNormIndexTemp = 0;
                                        }
                                    }
                                    else if(whichPart == 1)
                                    {
                                        if(vertPart != L"")
                                        {
                                            wstringToInt >> vertTCIndexTemp;
                                            vertTCIndexTemp -= 1;
                                        }
                                        else
                                            vertTCIndexTemp = 0;
                                        if(j == VertDef.length()-1)
                                            vertNormIndexTemp = 0;

                                    }                               
                                    else if(whichPart == 2)
                                    {
                                        std::wistringstream wstringToInt(vertPart);

                                        wstringToInt >> vertNormIndexTemp;
                                        vertNormIndexTemp -= 1;
                                    }

                                    vertPart = L"";
                                    whichPart++;                            
                                }
                            }                   

                            // Check for duplicate vertices
                            bool vertAlreadyExists = false;
                            if(totalVerts >= 3) // Make sure we at least have one triangle to check
                            {
                                for(int iCheck = 0; iCheck < totalVerts; ++iCheck)
                                {
                                    if(vertPosIndexTemp == vertPosIndex[iCheck] && !vertAlreadyExists)
                                    {
                                        if(vertTCIndexTemp == vertTCIndex[iCheck])
                                        {
                                            Model.Indices.push_back(iCheck);        // Set index for this vertex
                                            vertAlreadyExists = true;       // If we've made it here, the vertex already exists
                                        }
                                    }
                                }
                            }

                            if(!vertAlreadyExists)
                            {
                                vertPosIndex.push_back(vertPosIndexTemp);
                                vertTCIndex.push_back(vertTCIndexTemp);
                                vertNormIndex.push_back(vertNormIndexTemp);
                                totalVerts++;                       // New vertex created, add to total verts
                                Model.Indices.push_back(totalVerts-1);  // Set index for this vertex
                            }

                            // Set the second vertex for the next triangle to the last vertex we got        
                            lastVIndex = Model.Indices[vIndex]; // The last vertex index of this TRIANGLE

                            meshTriangles++;    // New triangle defined
                            vIndex++;       
                        }
                    }
                }
                break;

            case 'm': // mtllib - material library filename
                checkChar = fileIn.get();
                if(checkChar == 't')
                {
                    checkChar = fileIn.get();
                    if(checkChar == 'l')
                    {
                        checkChar = fileIn.get();
                        if(checkChar == 'l')
                        {
                            checkChar = fileIn.get();
                            if(checkChar == 'i')
                            {
                                checkChar = fileIn.get();
                                if(checkChar == 'b')
                                {
                                    checkChar = fileIn.get();
                                    if(checkChar == ' ')
                                    {
                                        // Store the material libraries file name
                                        fileIn >> meshMatLib;
                                    }
                                }
                            }
                        }
                    }
                }

                break;

            case 'u': // usemtl - which material to use
                checkChar = fileIn.get();
                if(checkChar == 's')
                {
                    checkChar = fileIn.get();
                    if(checkChar == 'e')
                    {
                        checkChar = fileIn.get();
                        if(checkChar == 'm')
                        {
                            checkChar = fileIn.get();
                            if(checkChar == 't')
                            {
                                checkChar = fileIn.get();
                                if(checkChar == 'l')
                                {
                                    checkChar = fileIn.get();
                                    if(checkChar == ' ')
                                    {
                                        meshMaterialsTemp = L"";  // Make sure this is cleared

                                        fileIn >> meshMaterialsTemp; // Get next type (string)

                                        meshMaterials.push_back(meshMaterialsTemp);
                                    }
                                }
                            }
                        }
                    }
                }
                break;

            default:                
                break;
            }
        }
    }
    else    // If we could not open the file
    {
        SwapChain->SetFullscreenState(false, NULL); // Make sure we are out of fullscreen

        // create message
        std::wstring message = L"Could not open: ";
        message += Filename;

        MessageBox(0, message.c_str(),  // Display message
            L"Error", MB_OK);

        return false;
    }

    Model.SubsetIndexStart.push_back(vIndex); // There won't be another index start after our last subset, so set it here

    // sometimes "g" is defined at the very top of the file, then again before the first group of faces.
    // This makes sure the first subset does not conatain "0" indices.
    if(Model.SubsetIndexStart[1] == 0)
    {
        Model.SubsetIndexStart.erase(Model.SubsetIndexStart.begin()+1);
        Model.Subsets--;
    }

    // Make sure we have a default for the tex coord and normal
    // if one or both are not specified
    if(!hasNorm)
        vertNorm.push_back(XMFLOAT3(0.0f, 0.0f, 0.0f));
    if(!hasTexCoord)
        vertTexCoord.push_back(XMFLOAT2(0.0f, 0.0f));

    // Close the obj file, and open the mtl file
    fileIn.close();
    fileIn.open(meshMatLib.c_str());

    std::wstring lastStringRead;
    int matCount = material.size(); // Total materials

    if (fileIn)
    {
        while(fileIn)
        {
            checkChar = fileIn.get();   // Get next char

            switch (checkChar)
            {
                // Check for comment
            case '#':
                checkChar = fileIn.get();
                while(checkChar != '\n')
                    checkChar = fileIn.get();
                break;

                // Set the colors
            case 'K':
                checkChar = fileIn.get();
                if(checkChar == 'd')  // Diffuse Color
                {
                    checkChar = fileIn.get();   // Remove space

                    fileIn >> material[matCount-1].Diffuse.x;
                    fileIn >> material[matCount-1].Diffuse.y;
                    fileIn >> material[matCount-1].Diffuse.z;
                }

                if(checkChar == 'a')  // Ambient Color
                {                   
                    checkChar = fileIn.get();   // Remove space

                    fileIn >> material[matCount-1].Ambient.x;
                    fileIn >> material[matCount-1].Ambient.y;
                    fileIn >> material[matCount-1].Ambient.z;
                }

                if(checkChar == 's')  // Ambient Color
                {                   
                    checkChar = fileIn.get();   // Remove space

                    fileIn >> material[matCount-1].Specular.x;
                    fileIn >> material[matCount-1].Specular.y;
                    fileIn >> material[matCount-1].Specular.z;
                }
                break;
                
            case 'N':
                checkChar = fileIn.get();

                if(checkChar == 's')  // Specular Power (Coefficient)
                {                   
                    checkChar = fileIn.get();   // Remove space

                    fileIn >> material[matCount-1].Specular.w;
                }

                break;

                // Check for transparency
            case 'T':
                checkChar = fileIn.get();
                if(checkChar == 'r')
                {
                    checkChar = fileIn.get();   // Remove space
                    float Transparency;
                    fileIn >> Transparency;

                    material[matCount-1].Diffuse.w = Transparency;

                    if(Transparency > 0.0f)
                        material[matCount-1].IsTransparent = true;
                }
                break;

                // Some obj files specify d for transparency
            case 'd':
                checkChar = fileIn.get();
                if(checkChar == ' ')
                {
                    float Transparency;
                    fileIn >> Transparency;

                    // 'd' - 0 being most transparent, and 1 being opaque, opposite of Tr
                    Transparency = 1.0f - Transparency;

                    material[matCount-1].Diffuse.w = Transparency;

                    if(Transparency > 0.0f)
                        material[matCount-1].IsTransparent = true;                  
                }
                break;

                // Get the diffuse map (texture)
            case 'm':
                checkChar = fileIn.get();
                if(checkChar == 'a')
                {
                    checkChar = fileIn.get();
                    if(checkChar == 'p')
                    {
                        checkChar = fileIn.get();
                        if(checkChar == '_')
                        {
                            // map_Kd - Diffuse map
                            checkChar = fileIn.get();
                            if(checkChar == 'K')
                            {
                                checkChar = fileIn.get();
                                if(checkChar == 'd')
                                {
                                    std::wstring fileNamePath;

                                    fileIn.get();   // Remove whitespace between map_Kd and file

                                    // Get the file path - We read the pathname char by char since
                                    // pathnames can sometimes contain spaces, so we will read until
                                    // we find the file extension
                                    bool texFilePathEnd = false;
                                    while(!texFilePathEnd)
                                    {
                                        checkChar = fileIn.get();

                                        fileNamePath += checkChar;

                                        if(checkChar == '.')
                                        {
                                            for(int i = 0; i < 3; ++i)
                                                fileNamePath += fileIn.get();

                                            texFilePathEnd = true;
                                        }                           
                                    }

                                    //check if this texture has already been loaded
                                    bool alreadyLoaded = false;
                                    for(int i = 0; i < TexMgr.TextureNameArray.size(); ++i)
                                    {
                                        if(fileNamePath == TexMgr.TextureNameArray[i])
                                        {
                                            alreadyLoaded = true;
                                            material[matCount-1].DiffuseTextureID = i;
                                            material[matCount-1].HasDiffTexture = true;
                                        }
                                    }

                                    //if the texture is not already loaded, load it now
                                    if(!alreadyLoaded)
                                    {
                                        ID3D11ShaderResourceView* tempSRV;
                                        hr = D3DX11CreateShaderResourceViewFromFile( device, fileNamePath.c_str(),
                                            NULL, NULL, &tempSRV, NULL );
                                        if(SUCCEEDED(hr))
                                        {
                                            TexMgr.TextureNameArray.push_back(fileNamePath.c_str());
                                            material[matCount-1].DiffuseTextureID = TexMgr.TextureList.size();
                                            TexMgr.TextureList.push_back(tempSRV);
                                            material[matCount-1].HasDiffTexture = true;
                                        }
                                    }   
                                }

                                // Get Ambient Map (texture)
                                if(checkChar == 'a')
                                {
                                    std::wstring fileNamePath;

                                    fileIn.get();   // Remove whitespace between map_Kd and file

                                    // Get the file path - We read the pathname char by char since
                                    // pathnames can sometimes contain spaces, so we will read until
                                    // we find the file extension
                                    bool texFilePathEnd = false;
                                    while(!texFilePathEnd)
                                    {
                                        checkChar = fileIn.get();

                                        fileNamePath += checkChar;

                                        if(checkChar == '.')
                                        {
                                            for(int i = 0; i < 3; ++i)
                                                fileNamePath += fileIn.get();

                                            texFilePathEnd = true;
                                        }                           
                                    }

                                    //check if this texture has already been loaded
                                    bool alreadyLoaded = false;
                                    for(int i = 0; i < TexMgr.TextureNameArray.size(); ++i)
                                    {
                                        if(fileNamePath == TexMgr.TextureNameArray[i])
                                        {
                                            alreadyLoaded = true;
                                            material[matCount-1].AmbientTextureID = i;
                                            material[matCount-1].HasAmbientTexture = true;
                                        }
                                    }

                                    //if the texture is not already loaded, load it now
                                    if(!alreadyLoaded)
                                    {
                                        ID3D11ShaderResourceView* tempSRV;
                                        hr = D3DX11CreateShaderResourceViewFromFile( device, fileNamePath.c_str(),
                                            NULL, NULL, &tempSRV, NULL );
                                        if(SUCCEEDED(hr))
                                        {
                                            TexMgr.TextureNameArray.push_back(fileNamePath.c_str());
                                            material[matCount-1].AmbientTextureID = TexMgr.TextureList.size();
                                            TexMgr.TextureList.push_back(tempSRV);
                                            material[matCount-1].HasAmbientTexture = true;
                                        }
                                    }   
                                }

                                // Get Specular Map (texture)
                                if(checkChar == 's')
                                {
                                    std::wstring fileNamePath;

                                    fileIn.get();   // Remove whitespace between map_Ks and file

                                    // Get the file path - We read the pathname char by char since
                                    // pathnames can sometimes contain spaces, so we will read until
                                    // we find the file extension
                                    bool texFilePathEnd = false;
                                    while(!texFilePathEnd)
                                    {
                                        checkChar = fileIn.get();

                                        fileNamePath += checkChar;

                                        if(checkChar == '.')
                                        {
                                            for(int i = 0; i < 3; ++i)
                                                fileNamePath += fileIn.get();

                                            texFilePathEnd = true;
                                        }                           
                                    }

                                    //check if this texture has already been loaded
                                    bool alreadyLoaded = false;
                                    for(int i = 0; i < TexMgr.TextureNameArray.size(); ++i)
                                    {
                                        if(fileNamePath == TexMgr.TextureNameArray[i])
                                        {
                                            alreadyLoaded = true;
                                            material[matCount-1].SpecularTextureID = i;
                                            material[matCount-1].HasSpecularTexture = true;
                                        }
                                    }

                                    //if the texture is not already loaded, load it now
                                    if(!alreadyLoaded)
                                    {
                                        ID3D11ShaderResourceView* tempSRV;
                                        hr = D3DX11CreateShaderResourceViewFromFile( device, fileNamePath.c_str(),
                                            NULL, NULL, &tempSRV, NULL );
                                        if(SUCCEEDED(hr))
                                        {
                                            TexMgr.TextureNameArray.push_back(fileNamePath.c_str());
                                            material[matCount-1].SpecularTextureID = TexMgr.TextureList.size();
                                            TexMgr.TextureList.push_back(tempSRV);
                                            material[matCount-1].HasSpecularTexture = true;
                                        }
                                    }   
                                }
                            }
                            
                            //map_d - alpha map
                            else if(checkChar == 'd')
                            {
                                std::wstring fileNamePath;

                                fileIn.get();   // Remove whitespace between map_Ks and file

                                // Get the file path - We read the pathname char by char since
                                // pathnames can sometimes contain spaces, so we will read until
                                // we find the file extension
                                bool texFilePathEnd = false;
                                while(!texFilePathEnd)
                                {
                                    checkChar = fileIn.get();

                                    fileNamePath += checkChar;

                                    if(checkChar == '.')
                                    {
                                        for(int i = 0; i < 3; ++i)
                                            fileNamePath += fileIn.get();

                                        texFilePathEnd = true;
                                    }                           
                                }

                                //check if this texture has already been loaded
                                bool alreadyLoaded = false;
                                for(int i = 0; i < TexMgr.TextureNameArray.size(); ++i)
                                {
                                    if(fileNamePath == TexMgr.TextureNameArray[i])
                                    {
                                        alreadyLoaded = true;
                                        material[matCount-1].AlphaTextureID = i;
                                        material[matCount-1].IsTransparent = true;
                                    }
                                }

                                //if the texture is not already loaded, load it now
                                if(!alreadyLoaded)
                                {
                                    ID3D11ShaderResourceView* tempSRV;
                                    hr = D3DX11CreateShaderResourceViewFromFile( device, fileNamePath.c_str(),
                                        NULL, NULL, &tempSRV, NULL );
                                    if(SUCCEEDED(hr))
                                    {
                                        TexMgr.TextureNameArray.push_back(fileNamePath.c_str());
                                        material[matCount-1].AlphaTextureID = TexMgr.TextureList.size();
                                        TexMgr.TextureList.push_back(tempSRV);
                                        material[matCount-1].IsTransparent = true;
                                    }
                                }   
                            }

                            // map_bump - bump map (Normal Map)
                            else if(checkChar == 'b')
                            {
                                checkChar = fileIn.get();
                                if(checkChar == 'u')
                                {
                                    checkChar = fileIn.get();
                                    if(checkChar == 'm')
                                    {
                                        checkChar = fileIn.get();
                                        if(checkChar == 'p')
                                        {
                                            std::wstring fileNamePath;

                                            fileIn.get();   // Remove whitespace between map_bump and file

                                            // Get the file path - We read the pathname char by char since
                                            // pathnames can sometimes contain spaces, so we will read until
                                            // we find the file extension
                                            bool texFilePathEnd = false;
                                            while(!texFilePathEnd)
                                            {
                                                checkChar = fileIn.get();

                                                fileNamePath += checkChar;

                                                if(checkChar == '.')
                                                {
                                                    for(int i = 0; i < 3; ++i)
                                                        fileNamePath += fileIn.get();

                                                    texFilePathEnd = true;
                                                }                           
                                            }

                                            //check if this texture has already been loaded
                                            bool alreadyLoaded = false;
                                            for(int i = 0; i < TexMgr.TextureNameArray.size(); ++i)
                                            {
                                                if(fileNamePath == TexMgr.TextureNameArray[i])
                                                {
                                                    alreadyLoaded = true;
                                                    material[matCount-1].NormMapTextureID = i;
                                                    material[matCount-1].HasNormMap = true;
                                                }
                                            }

                                            //if the texture is not already loaded, load it now
                                            if(!alreadyLoaded)
                                            {
                                                ID3D11ShaderResourceView* tempSRV;
                                                hr = D3DX11CreateShaderResourceViewFromFile( device, fileNamePath.c_str(),
                                                    NULL, NULL, &tempSRV, NULL );
                                                if(SUCCEEDED(hr))
                                                {
                                                    TexMgr.TextureNameArray.push_back(fileNamePath.c_str());
                                                    material[matCount-1].NormMapTextureID = TexMgr.TextureList.size();
                                                    TexMgr.TextureList.push_back(tempSRV);
                                                    material[matCount-1].HasNormMap = true;
                                                }
                                            }   
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                break;

            case 'n': // newmtl - Declare new material
                checkChar = fileIn.get();
                if(checkChar == 'e')
                {
                    checkChar = fileIn.get();
                    if(checkChar == 'w')
                    {
                        checkChar = fileIn.get();
                        if(checkChar == 'm')
                        {
                            checkChar = fileIn.get();
                            if(checkChar == 't')
                            {
                                checkChar = fileIn.get();
                                if(checkChar == 'l')
                                {
                                    checkChar = fileIn.get();
                                    if(checkChar == ' ')
                                    {
                                        // New material, set its defaults
                                        SurfaceMaterial tempMat;
                                        material.push_back(tempMat);
                                        fileIn >> material[matCount].MatName;
                                        material[matCount].IsTransparent = false;
                                        material[matCount].HasDiffTexture = false;
                                        material[matCount].HasAmbientTexture = false;
                                        material[matCount].HasSpecularTexture = false;
                                        material[matCount].HasAlphaTexture = false;
                                        material[matCount].HasNormMap = false;
                                        material[matCount].NormMapTextureID = 0;
                                        material[matCount].DiffuseTextureID = 0;
                                        material[matCount].AlphaTextureID = 0;
                                        material[matCount].SpecularTextureID = 0;
                                        material[matCount].AmbientTextureID = 0;
                                        material[matCount].Specular = XMFLOAT4(0,0,0,0);
                                        material[matCount].Ambient = XMFLOAT3(0,0,0);
                                        material[matCount].Diffuse = XMFLOAT4(0,0,0,0);
                                        matCount++;
                                    }
                                }
                            }
                        }
                    }
                }
                break;

            default:
                break;
            }
        }
    }   
    else    // If we could not open the material library
    {
        SwapChain->SetFullscreenState(false, NULL); // Make sure we are out of fullscreen

        std::wstring message = L"Could not open: ";
        message += meshMatLib;

        MessageBox(0, message.c_str(),
            L"Error", MB_OK);

        return false;
    }

    // Set the subsets material to the index value
    // of the its material in our material array
    for(int i = 0; i < Model.Subsets; ++i)
    {
        bool hasMat = false;
        for(int j = 0; j < material.size(); ++j)
        {
            if(meshMaterials[i] == material[j].MatName)
            {
                Model.SubsetMaterialID.push_back(j);
                hasMat = true;
            }
        }
        if(!hasMat)
            Model.SubsetMaterialID.push_back(0); // Use first material in array
    }

    std::vector<Vertex> vertices;
    Vertex tempVert;

    // Create our vertices using the information we got 
    // from the file and store them in a vector
    for(int j = 0 ; j < totalVerts; ++j)
    {
        tempVert.pos = vertPos[vertPosIndex[j]];
        tempVert.normal = vertNorm[vertNormIndex[j]];
        tempVert.texCoord = vertTexCoord[vertTCIndex[j]];

        vertices.push_back(tempVert);
        Model.Vertices.push_back(tempVert.pos);
    }

    //If computeNormals was set to true then we will create our own
    //normals, if it was set to false we will use the obj files normals
    if(ComputeNormals)
    {
        std::vector<XMFLOAT3> tempNormal;

        //normalized and unnormalized normals
        XMFLOAT3 unnormalized = XMFLOAT3(0.0f, 0.0f, 0.0f);

        //tangent stuff
        std::vector<XMFLOAT3> tempTangent;
        XMFLOAT3 tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
        float tcU1, tcV1, tcU2, tcV2;

        //Used to get vectors (sides) from the position of the verts
        float vecX, vecY, vecZ;

        //Two edges of our triangle
        XMVECTOR edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        XMVECTOR edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

        //Compute face normals
        //And Tangents
        for(int i = 0; i < meshTriangles; ++i)
        {
            //Get the vector describing one edge of our triangle (edge 0,2)
            vecX = vertices[Model.Indices[(i*3)]].pos.x - vertices[Model.Indices[(i*3)+2]].pos.x;
            vecY = vertices[Model.Indices[(i*3)]].pos.y - vertices[Model.Indices[(i*3)+2]].pos.y;
            vecZ = vertices[Model.Indices[(i*3)]].pos.z - vertices[Model.Indices[(i*3)+2]].pos.z;       
            edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f);    //Create our first edge

            //Get the vector describing another edge of our triangle (edge 2,1)
            vecX = vertices[Model.Indices[(i*3)+2]].pos.x - vertices[Model.Indices[(i*3)+1]].pos.x;
            vecY = vertices[Model.Indices[(i*3)+2]].pos.y - vertices[Model.Indices[(i*3)+1]].pos.y;
            vecZ = vertices[Model.Indices[(i*3)+2]].pos.z - vertices[Model.Indices[(i*3)+1]].pos.z;     
            edge2 = XMVectorSet(vecX, vecY, vecZ, 0.0f);    //Create our second edge

            //Cross multiply the two edge vectors to get the un-normalized face normal
            XMStoreFloat3(&unnormalized, XMVector3Cross(edge1, edge2));

            tempNormal.push_back(unnormalized);

            //Find first texture coordinate edge 2d vector
            tcU1 = vertices[Model.Indices[(i*3)]].texCoord.x - vertices[Model.Indices[(i*3)+2]].texCoord.x;
            tcV1 = vertices[Model.Indices[(i*3)]].texCoord.y - vertices[Model.Indices[(i*3)+2]].texCoord.y;

            //Find second texture coordinate edge 2d vector
            tcU2 = vertices[Model.Indices[(i*3)+2]].texCoord.x - vertices[Model.Indices[(i*3)+1]].texCoord.x;
            tcV2 = vertices[Model.Indices[(i*3)+2]].texCoord.y - vertices[Model.Indices[(i*3)+1]].texCoord.y;

            //Find tangent using both tex coord edges and position edges
            tangent.x = (tcV1 * XMVectorGetX(edge1) - tcV2 * XMVectorGetX(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
            tangent.y = (tcV1 * XMVectorGetY(edge1) - tcV2 * XMVectorGetY(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
            tangent.z = (tcV1 * XMVectorGetZ(edge1) - tcV2 * XMVectorGetZ(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));

            tempTangent.push_back(tangent);
        }

        //Compute vertex normals (normal Averaging)
        XMVECTOR normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        XMVECTOR tangentSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        int facesUsing = 0;
        float tX, tY, tZ;   //temp axis variables

        //Go through each vertex
        for(int i = 0; i < totalVerts; ++i)
        {
            //Check which triangles use this vertex
            for(int j = 0; j < meshTriangles; ++j)
            {
                if(Model.Indices[j*3] == i ||
                    Model.Indices[(j*3)+1] == i ||
                    Model.Indices[(j*3)+2] == i)
                {
                    tX = XMVectorGetX(normalSum) + tempNormal[j].x;
                    tY = XMVectorGetY(normalSum) + tempNormal[j].y;
                    tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;

                    normalSum = XMVectorSet(tX, tY, tZ, 0.0f);  //If a face is using the vertex, add the unormalized face normal to the normalSum
    
                    //We can reuse tX, tY, tZ to sum up tangents
                    tX = XMVectorGetX(tangentSum) + tempTangent[j].x;
                    tY = XMVectorGetY(tangentSum) + tempTangent[j].y;
                    tZ = XMVectorGetZ(tangentSum) + tempTangent[j].z;

                    tangentSum = XMVectorSet(tX, tY, tZ, 0.0f); //sum up face tangents using this vertex

                    facesUsing++;
                }
            }

            //Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
            normalSum = normalSum / facesUsing;
            tangentSum = tangentSum / facesUsing;

            //Normalize the normalSum vector and tangent
            normalSum = XMVector3Normalize(normalSum);
            tangentSum =  XMVector3Normalize(tangentSum);

            //Store the normal and tangent in our current vertex
            vertices[i].normal.x = XMVectorGetX(normalSum);
            vertices[i].normal.y = XMVectorGetY(normalSum);
            vertices[i].normal.z = XMVectorGetZ(normalSum);

            vertices[i].tangent.x = XMVectorGetX(tangentSum);
            vertices[i].tangent.y = XMVectorGetY(tangentSum);
            vertices[i].tangent.z = XMVectorGetZ(tangentSum);

            //Clear normalSum, tangentSum and facesUsing for next vertex
            normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
            tangentSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
            facesUsing = 0;

        }
    }

	// Create Axis-Aligned Bounding Box (AABB)
	XMFLOAT3 minVertex = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 maxVertex = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	Model.BoundingSphere = 0;
	
	for(UINT i = 0; i < Model.Vertices.size(); i++)
	{		
		// The minVertex and maxVertex will most likely not be actual vertices in the model, but vertices
		// that use the smallest and largest x, y, and z values from the model to be sure ALL vertices are
		// covered by the bounding volume

		//Get the smallest vertex 
		minVertex.x = min(minVertex.x, Model.Vertices[i].x);	// Find smallest x value in model
		minVertex.y = min(minVertex.y, Model.Vertices[i].y);	// Find smallest y value in model
		minVertex.z = min(minVertex.z, Model.Vertices[i].z);	// Find smallest z value in model

		//Get the largest vertex 
		maxVertex.x = max(maxVertex.x, Model.Vertices[i].x);	// Find largest x value in model
		maxVertex.y = max(maxVertex.y, Model.Vertices[i].y);	// Find largest y value in model
		maxVertex.z = max(maxVertex.z, Model.Vertices[i].z);	// Find largest z value in model
	}	
	
	// Our AABB [0] is the min vertex and [1] is the max
	Model.AABB.push_back(minVertex);
	Model.AABB.push_back(maxVertex);
	
	// Get models true center
	Model.Center.x = maxVertex.x - minVertex.x / 2.0f;
	Model.Center.y = maxVertex.y - minVertex.y / 2.0f;
	Model.Center.z = maxVertex.z - minVertex.z / 2.0f;

	// Now that we have the center, get the bounding sphere	
	for(UINT i = 0; i < Model.Vertices.size(); i++)
	{		
		float x = (Model.Center.x - Model.Vertices[i].x) * (Model.Center.x - Model.Vertices[i].x);
		float y = (Model.Center.y - Model.Vertices[i].y) * (Model.Center.y - Model.Vertices[i].y);
		float z = (Model.Center.z - Model.Vertices[i].z) * (Model.Center.z - Model.Vertices[i].z);

		// Get models bounding sphere
		Model.BoundingSphere = max(Model.BoundingSphere, (x+y+z));
	}

	// We didn't use the square root when finding the largest distance since it slows things down.
	// We can square root the answer from above to get the actual bounding sphere now
	Model.BoundingSphere = sqrt(Model.BoundingSphere);


    //Create index buffer
    D3D11_BUFFER_DESC indexBufferDesc;
    ZeroMemory( &indexBufferDesc, sizeof(indexBufferDesc) );

    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(DWORD) * meshTriangles*3;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA iinitData;

    iinitData.pSysMem = &Model.Indices[0];
    device->CreateBuffer(&indexBufferDesc, &iinitData, &Model.IndexBuff);

    //Create Vertex Buffer
    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory( &vertexBufferDesc, sizeof(vertexBufferDesc) );

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof( Vertex ) * totalVerts;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexBufferData; 

    ZeroMemory( &vertexBufferData, sizeof(vertexBufferData) );
    vertexBufferData.pSysMem = &vertices[0];
    hr = device->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &Model.VertBuff);

    return true;
}
///////////////**************new**************////////////////////

bool InitScene()
{
    ///////////////**************new**************////////////////////
    if(!LoadObjModel(d3d11Device, L"ground.obj", groundModel, material, textureMgr, true, true))
        return false;
    ///////////////**************new**************////////////////////

    // Compile Shaders from shader file
    hr = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "VS", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
    hr = D3DX11CompileFromFile(L"Effects.fx", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0);

    // Create the Shader Objects
    hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
    hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);

    // Set Vertex and Pixel Shaders
    d3d11DevCon->VSSetShader(VS, 0, 0);
    d3d11DevCon->PSSetShader(PS, 0, 0);

    light.pos = XMFLOAT3(0.0f, 7.0f, 0.0f);
    light.dir = XMFLOAT3(0.5f, 0.75f, -0.5f);
    light.range = 1000.0f;
    light.cone = 12.0f;
    light.att = XMFLOAT3(0.4f, 0.02f, 0.000f);
    light.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    light.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    // Create the Input Layout
    hr = d3d11Device->CreateInputLayout( layout, numElements, VS_Buffer->GetBufferPointer(), 
        VS_Buffer->GetBufferSize(), &vertLayout );

    // Set the Input Layout
    d3d11DevCon->IASetInputLayout( vertLayout );

    // Set Primitive Topology
    d3d11DevCon->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Create the Viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = Width;
    viewport.Height = Height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    // Set the Viewport
    d3d11DevCon->RSSetViewports(1, &viewport);

    // Create the buffer to send to the cbuffer in effect file
    D3D11_BUFFER_DESC cbbd; 
    ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

    cbbd.Usage = D3D11_USAGE_DEFAULT;
    cbbd.ByteWidth = sizeof(cbPerObject);
    cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbbd.CPUAccessFlags = 0;
    cbbd.MiscFlags = 0;

    hr = d3d11Device->CreateBuffer(&cbbd, NULL, &cbPerObjectBuffer);

    // Create the buffer to send to the cbuffer per frame in effect file
    ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));

    cbbd.Usage = D3D11_USAGE_DEFAULT;
    cbbd.ByteWidth = sizeof(cbPerFrame);
    cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbbd.CPUAccessFlags = 0;
    cbbd.MiscFlags = 0;

    hr = d3d11Device->CreateBuffer(&cbbd, NULL, &cbPerFrameBuffer);

    // Camera information
    camPosition = XMVectorSet( 0.0f, 5.0f, -8.0f, 0.0f );
    camTarget = XMVectorSet( 0.0f, 0.5f, 0.0f, 0.0f );
    camUp = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

    // Set the View matrix
    camView = XMMatrixLookAtLH( camPosition, camTarget, camUp );

    // Set the Projection matrix
    camProjection = XMMatrixPerspectiveFovLH( 0.4f*3.14f, Width/Height, 1.0f, 1000.0f);

    // Describe the Sample State
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;    
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    //Create the Sample State
    hr = d3d11Device->CreateSamplerState( &sampDesc, &LinearSamplerState );

    D3D11_RASTERIZER_DESC cmdesc;

    ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
    cmdesc.FillMode = D3D11_FILL_SOLID;
    cmdesc.CullMode = D3D11_CULL_BACK;
    cmdesc.FrontCounterClockwise = true;
    hr = d3d11Device->CreateRasterizerState(&cmdesc, &CCWcullMode);

    cmdesc.FrontCounterClockwise = false;

    hr = d3d11Device->CreateRasterizerState(&cmdesc, &CWcullMode);

    cmdesc.CullMode = D3D11_CULL_NONE;
    //cmdesc.FillMode = D3D11_FILL_WIREFRAME;
    hr = d3d11Device->CreateRasterizerState(&cmdesc, &RSCullNone);

    D3D11_BLEND_DESC blendDesc;
    ZeroMemory( &blendDesc, sizeof(blendDesc) );

    D3D11_RENDER_TARGET_BLEND_DESC rtbd;
    ZeroMemory( &rtbd, sizeof(rtbd) );

    rtbd.BlendEnable             = true;
    rtbd.SrcBlend                = D3D11_BLEND_INV_SRC_ALPHA;
    rtbd.DestBlend               = D3D11_BLEND_SRC_ALPHA;
    rtbd.BlendOp                 = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha           = D3D11_BLEND_INV_SRC_ALPHA;
    rtbd.DestBlendAlpha          = D3D11_BLEND_SRC_ALPHA;
    rtbd.BlendOpAlpha            = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask   = D3D10_COLOR_WRITE_ENABLE_ALL;

    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;

    d3d11Device->CreateBlendState(&blendDesc, &Transparency);

    return true;
}

void StartTimer()
{
    LARGE_INTEGER frequencyCount;
    QueryPerformanceFrequency(&frequencyCount);

    countsPerSecond = double(frequencyCount.QuadPart);

    QueryPerformanceCounter(&frequencyCount);
    CounterStart = frequencyCount.QuadPart;
}

double GetTime()
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    return double(currentTime.QuadPart-CounterStart)/countsPerSecond;
}

double GetFrameTime()
{
    LARGE_INTEGER currentTime;
    __int64 tickCount;
    QueryPerformanceCounter(&currentTime);

    tickCount = currentTime.QuadPart-frameTimeOld;
    frameTimeOld = currentTime.QuadPart;

    if(tickCount < 0.0f)
        tickCount = 0.0f;

    return float(tickCount)/countsPerSecond;
}

void UpdateScene(double time)
{
    //the loaded models world space
    groundModel.World = XMMatrixIdentity();

    Rotation = XMMatrixRotationY(3.14f);
    Scale = XMMatrixScaling( 1.0f, 1.0f, 1.0f );
    Translation = XMMatrixTranslation( 0.0f, 0.0f, 0.0f );

    groundModel.World = Rotation * Scale * Translation;
}

void DrawScene()
{
    // Clear our render target and depth/stencil view
    float bgColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);  
    d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Update the cbPerFrame
    constbuffPerFrame.light = light;
    d3d11DevCon->UpdateSubresource( cbPerFrameBuffer, 0, NULL, &constbuffPerFrame, 0, 0 );
    d3d11DevCon->PSSetConstantBuffers(0, 1, &cbPerFrameBuffer); 

    // Set our Render Target
    d3d11DevCon->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );

    // Set the default blend state (no blending) for opaque objects
    d3d11DevCon->OMSetBlendState(0, 0, 0xffffffff);

    // Set Vertex and Pixel Shaders
    d3d11DevCon->VSSetShader(VS, 0, 0);
    d3d11DevCon->PSSetShader(PS, 0, 0);

    d3d11DevCon->PSSetSamplers( 0, 1, &LinearSamplerState );
    d3d11DevCon->RSSetState(RSCullNone);

    /////Draw our model's NON-transparent subsets/////
    // Only set the vert and index buffer once per model
    // Set the grounds index buffer
    d3d11DevCon->IASetIndexBuffer( groundModel.IndexBuff, DXGI_FORMAT_R32_UINT, 0);
    // Set the grounds vertex buffer
    d3d11DevCon->IASetVertexBuffers( 0, 1, &groundModel.VertBuff, &stride, &offset );

    // Set the WVP matrix and send it to the constant buffer in effect file
    // This also only needs to be set once per model
    WVP = groundModel.World * camView * camProjection;
    cbPerObj.WVP = XMMatrixTranspose(WVP);  
    cbPerObj.World = XMMatrixTranspose(groundModel.World);

    for(int i = 0; i < groundModel.Subsets; ++i)
    {   
        // Only draw the NON-transparent parts of the model. 
        if(!material[groundModel.SubsetMaterialID[i]].IsTransparent)            
        {
            cbPerObj.difColor = material[groundModel.SubsetMaterialID[i]].Diffuse;      // Let shader know which color to draw the model 
                                                                                        // (if no diffuse texture we defined)
            cbPerObj.hasTexture = material[groundModel.SubsetMaterialID[i]].HasDiffTexture; // Let shader know if we need to use a texture
            cbPerObj.hasNormMap = material[groundModel.SubsetMaterialID[i]].HasNormMap; // Let shader know if we need to do normal mapping
            d3d11DevCon->UpdateSubresource( cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0 );
            d3d11DevCon->VSSetConstantBuffers( 0, 1, &cbPerObjectBuffer );
            d3d11DevCon->PSSetConstantBuffers( 1, 1, &cbPerObjectBuffer );

            // If this subset has a diffuse texture, send it to the pixel shader
            if(material[groundModel.SubsetMaterialID[i]].HasDiffTexture)    
                d3d11DevCon->PSSetShaderResources( 0, 1, &textureMgr.TextureList[material[groundModel.SubsetMaterialID[i]].DiffuseTextureID] );

            // If this subset has a normal (bump) map, send it to the pixel shader
            if(material[groundModel.SubsetMaterialID[i]].HasNormMap)    
                d3d11DevCon->PSSetShaderResources( 1, 1, &textureMgr.TextureList[material[groundModel.SubsetMaterialID[i]].NormMapTextureID] );     

            // Draw the NON-transparent stuff
            int indexStart = groundModel.SubsetIndexStart[i];
            int indexDrawAmount =  groundModel.SubsetIndexStart[i+1] - indexStart;
            d3d11DevCon->DrawIndexed( indexDrawAmount, indexStart, 0 );     
        }
    }



    // Draw other opaque objects (non-transparent)



    // Now after all the non transparent objects have been drawn, draw the transparent stuff



    /////Draw our model's TRANSPARENT subsets/////
    d3d11DevCon->IASetIndexBuffer( groundModel.IndexBuff, DXGI_FORMAT_R32_UINT, 0);
    d3d11DevCon->IASetVertexBuffers( 0, 1, &groundModel.VertBuff, &stride, &offset );

    WVP = groundModel.World * camView * camProjection;
    cbPerObj.WVP = XMMatrixTranspose(WVP);  
    cbPerObj.World = XMMatrixTranspose(groundModel.World);

    for(int i = 0; i < groundModel.Subsets; ++i)
    {   
        // Only draw the TRANSPARENT parts of the model now. 
        if(material[groundModel.SubsetMaterialID[i]].IsTransparent)                     
        {
            cbPerObj.difColor = material[groundModel.SubsetMaterialID[i]].Diffuse;      // Let shader know which color to draw the model 
                                                                                        // (if no diffuse texture we defined)
            cbPerObj.hasTexture = material[groundModel.SubsetMaterialID[i]].HasDiffTexture; // Let shader know if we need to use a texture
            cbPerObj.hasNormMap = material[groundModel.SubsetMaterialID[i]].HasNormMap; // Let shader know if we need to do normal mapping
            d3d11DevCon->UpdateSubresource( cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0 );
            d3d11DevCon->VSSetConstantBuffers( 0, 1, &cbPerObjectBuffer );
            d3d11DevCon->PSSetConstantBuffers( 1, 1, &cbPerObjectBuffer );

            // If this subset has a diffuse texture, send it to the pixel shader
            if(material[groundModel.SubsetMaterialID[i]].HasDiffTexture)                    
                d3d11DevCon->PSSetShaderResources( 0, 1, &textureMgr.TextureList[material[groundModel.SubsetMaterialID[i]].DiffuseTextureID] );

            // If this subset has a normal (bump) map, send it to the pixel shader
            if(material[groundModel.SubsetMaterialID[i]].HasNormMap)                    
                d3d11DevCon->PSSetShaderResources( 1, 1, &textureMgr.TextureList[material[groundModel.SubsetMaterialID[i]].NormMapTextureID] );     

            // The transparent parts are now drawn
            int indexStart = groundModel.SubsetIndexStart[i];
            int indexDrawAmount =  groundModel.SubsetIndexStart[i+1] - indexStart;
            d3d11DevCon->DrawIndexed( indexDrawAmount, indexStart, 0 );     
        }
    }

    //Present the backbuffer to the screen
    SwapChain->Present(0, 0);
}

int messageloop(){
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));
    while(true)
    {
        BOOL PeekMessageL( 
            LPMSG lpMsg,
            HWND hWnd,
            UINT wMsgFilterMin,
            UINT wMsgFilterMax,
            UINT wRemoveMsg
            );

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg); 
            DispatchMessage(&msg);
        }
        else{
            // run game code    
            frameCount++;
            if(GetTime() > 1.0f)
            {
                fps = frameCount;
                frameCount = 0;
                StartTimer();
            }   

            frameTime = GetFrameTime();

            DetectInput(frameTime);
            UpdateScene(frameTime);
            DrawScene();
        }
    }
    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch( msg )
    {
    case WM_KEYDOWN:
        if( wParam == VK_ESCAPE ){
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd,
        msg,
        wParam,
        lParam);
}