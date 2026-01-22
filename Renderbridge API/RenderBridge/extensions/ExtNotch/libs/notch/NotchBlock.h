#ifndef NOTCH_BLOCK_H
#define NOTCH_BLOCK_H

struct HINSTANCE__;
using HMODULE = HINSTANCE__*;
#include <string>
#include <vector>

class NotchBlock;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;

enum LogMessageType
{
	DebugType_Info = 0,
	DebugType_Warning,
	DebugType_Assert,
	DebugType_DebugText
};

enum ColourSpace
{
	ColourSpace_SRGBGamma22 = 0,
	ColourSpace_SRGBLinear = 1,
	ColourSpace_ACEScg = 2
};

typedef void (*CLoggingCallback)(int messageType, const char* file, int lineNo, const char* expression, const char* text);

// One-time create the DFX plugin.
typedef int (* _DemolitionCreate)(const char* commandLineParameters);

// Release the DFX plugin.
typedef int (* _DemolitionRelease)();

// Initialise the render device used by the plugin. Must be done before any resources are created.
typedef int (* _DemolitionInitRenderDeviceD3D11)(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

// Load the plugin's document and data.
typedef int (* _DemolitionLoadDFX)(const char* dfxFilename);

// Initialise an instance of the DFX. Copies the document / node structure and properties; shares resources.
typedef int (* _DemolitionInitialiseDFX)();

// Release an instance of the DFX
typedef int (* _DemolitionReleaseDFXInstance)(int handle);

// Deactivate the DFX - when we won't be rendering it for a while. Releases temporary render targets back to the pool. It will reactivate automatically when it renders again.
typedef int (* _DemolitionDeactivateDFX)();

// Purge the VRAM pool at block level, actually freeing the memory it holds that isn't in use. Call this when you definitely won't be rendering any instances of the block for a while.
typedef int (* _DemolitionPurgeVRAMPool)();

// Set the currently active DFX instance.
typedef int (* _DemolitionSetCurrentDocumentHandle)(int handle);

// Get the currently active DFX instance.
typedef int (* _DemolitionGetCurrentDocumentHandle)();

// Reset the state of the current DFX instance to the initial state.
typedef int (* _DemolitionResetState)();

// (For external clock) Set the current time and delta. 
typedef int (* _DemolitionSetTime)(double time, double timeDelta);

// Render one frame to a given texture and render target view. This method does not need to create any views to render.
typedef int (* _DemolitionRenderFrameToRenderTarget)(ID3D11Texture2D* texture, ID3D11RenderTargetView* renderTargetView);

// Get the GUID of the layer by index. guid must have at least 64 characters of space.
typedef int (* _DemolitionGetLayerGUID)(char* guid, int layerIndex);

// Get the index of the layer using the layer's GUID. returns -1 if not found.
typedef int (* _DemolitionGetLayerIndexByGUID)(const char* guid);

// Get the GUID of the current layer. guid must have at least 64 characters of space.
typedef int (* _DemolitionGetCurrentLayerGUID)(char* guid);

// Get the layer duration. Only valid when Layers As Separate Effects is disabled.
typedef int(*_DemolitionGetLayerDuration)(double *duration, const char *layerGUID);

// Get the block duration.
typedef int(*_DemolitionGetBlockDuration)(double *duration);

// Get the number of exposed properties.
typedef int (* _DemolitionGetNumProperties)();

// Get the unique ID of a property by index.
typedef int (* _DemolitionGetPropertyUniqueIdByIndex)(char* propertyUniqueId, int index);

// Get the name of a property by unique id.
typedef int (* _DemolitionGetPropertyNameById)(char* propertyName, const char* propertyUniqueId);

// Get the group name of a property by unique id.
typedef int (* _DemolitionGetPropertyGroupNameById)(char* propertyName, const char* propertyUniqueId);

// Get the type of a property by unique id.
typedef int (* _DemolitionGetPropertyTypeById)(char* propertyType, const char* propertyUniqueId);

// Get the list of enums associated with a property by unique id. enumNames must be at least 4096 bytes in size.
typedef int (* _DemolitionGetPropertyEnumsById)(char* enumNames, const char* propertyUniqueId);

// Get a property value as a string, by unique id.
typedef int (* _DemolitionGetPropertyValueById)(char* propertyValue, const char* propertyUniqueId);

// Set a property value as a string, by unique id.
typedef int (* _DemolitionSetPropertyValueById)(const char* propertyValue, const char* propertyUniqueId);

// Set a property value as a float, by index.
typedef int (* _DemolitionSetFloatPropertyValueByIndex)(float propertyValue, int propertyIndex);

// Set a property value as a float, by unique id
typedef int (* _DemolitionSetFloatPropertyValueById)(float propertyValue, const char* propertyUniqueId);

// Set a property value as a unicode string, by unique id.
typedef int (*_DemolitionSetUnicodePropertyValueById)(const wchar_t* propertyValue, const char* propertyUniqueId);

// Set a property value as a unicode string, by index.
typedef int (*_DemolitionSetUnicodePropertyValueByIndex)(const wchar_t* propertyValue, int propertyIndex);

// Get a property value as a unicode string, by unique id.
typedef int(*_DemolitionGetUnicodeStringPropertyValueById)(wchar_t* propertyValue, const char* propertyUniqueId);

// Get a property value as a unicode string, by index.
typedef int(*_DemolitionGetUnicodeStringPropertyValueByIndex)(wchar_t* propertyValue, int propertyIndex);

// Get a property's num array elements. Returns 0 if the property is not an array.
typedef int(*_DemolitionGetPropertyArrayElementCount)(const char* propertyUniqueId);

// Get a property range by unique id.
typedef int (* _DemolitionGetPropertyRangeById)(float* minValue, float* maxValue, const char* propertyUniqueId);

// Set a property value to the given texture, where texture is an ID3D11Texture2D* and srv is an ID3D11ShaderResourceView*. This method does not need to create any views.
typedef int (* _DemolitionSetPropertyDX11TextureSRVValueById)(void* texture, void* srv, const char* propertyUniqueId);

// Is a property used by the given layer?
typedef int (* _DemolitionIsPropertyActiveInLayer)(const char* propertyUniqueId, const char* layerGUID);

// Get the DMX offset hint for an exposed property, by unique ID. -1 if no hint exists for that property, otherwise a 0-based offset.
typedef int (* _DemolitionGetPropertyDMXOffset)(const char* propertyUniqueId);

// Get the DMX size hint for an exposed property, by unique ID. 0 = 8bit (1 DMX channel), 1 = 16bit (2 DMX channels).
typedef int (* _DemolitionGetPropertyDMX16Bit)(const char* propertyUniqueId);

// Should a property's value be read, rather than written?
typedef int (* _DemolitionGetPropertyReadable)(const char* propertyUniqueId);

// Set a float property value as an array of floats, by index
typedef int(*_DemolitionSetFloatArrayPropertyValueByIndex)(float* propertyValues, int propertyIndex, int maxNumFloats);

// Set a float property value as an array of floats, by unique id.
typedef int(*_DemolitionSetFloatArrayPropertyValueById)(float* propertyValues, const char* propertyUniqueId, int maxNumFloats);

// Get a float array of values, by unique id.
typedef int(*_DemolitionGetFloatArrayPropertyValueById)(float* propertyValues, const char* propertyUniqueId, int maxNumFloats);

// Get the project colour space.
typedef int(*_DemolitionGetProjectWorkingColourSpace)();

// Get the layer colour space.
typedef int(*_DemolitionGetLayerWorkingColourSpace)(const char* guid);

// Set the input/output colour space.
typedef int(*_DemolitionSetInputOutputColourSpace)(int colourSpace);

// Determine if the block has Layers As Separate Effects enabled or not. Returns 0 if not, 1 if so.
typedef int(*_DemolitionIsLayersAsSeparateEffectsEnabled)();

typedef int (* _DemolitionGetVersion)();
typedef unsigned __int64 (* _DemolitionGetVRAMUsage)();
typedef int (* _GetCreatorsString)(char* resultBuffer, int resultBufferSize);

// Sets the logging callback
typedef void (*_DemolitionSetLogCallbackC)(CLoggingCallback callbackFunction);

// Determine whether exposed property list has changed (via network editing) since last call to this function. Returns 0 if not, 1 if so.
typedef int (*_DemolitionHasExposedPropertiesChanged)();

// Determine whether this block is a proxy block. Returns 0 if not, 1 if so.
typedef int (*_DemolitionIsProxyBlock)();

class NotchExposedProperty 
{
public:
	enum PropertyType
	{
		PropertyType_Float = 0,
		PropertyType_Colour,
		PropertyType_Int,
		PropertyType_String,
		PropertyType_Texture,
		PropertyType_Resource,
		PropertyType_Enum,
		PropertyType_ExposedNull,
		PropertyType_ExposedCamera,
		PropertyType_PosRotScale,
		PropertyType_PosRotQuatScale,
		PropertyType_Matrix4x4
	};
	enum PropertyDataType
	{
		PropertyDataType_Float = 0,
		PropertyDataType_Int,
		PropertyDataType_String,
		PropertyDataType_Texture
	};

	NotchExposedProperty()
		: m_index(0)
		, m_readable(0)
	{
	}
	virtual ~NotchExposedProperty()
	{
	}
	virtual const NotchExposedProperty::PropertyType GetPropertyType() const = 0;
	virtual const NotchExposedProperty::PropertyDataType GetPropertyDataType() const = 0;
	
	std::string m_propertyId;
	std::string m_name;
	std::string m_groupName;
	std::string m_initialValue;
	int m_index;
	int m_readable;
};

class NotchExposedPropertyString : public NotchExposedProperty
{
public:
	NotchExposedPropertyString()
	{
		m_buffer[0] = 0;
	}
	virtual const NotchExposedProperty::PropertyType GetPropertyType() const { return PropertyType_String; }
	virtual const NotchExposedProperty::PropertyDataType GetPropertyDataType() const { return PropertyDataType_String; }

	std::string m_value;
	std::wstring m_unicodeValue;
	char m_buffer[1024];
};

class NotchExposedPropertyImage : public NotchExposedProperty
{
public:
	NotchExposedPropertyImage()
		: m_texture(NULL)
		, m_srv(NULL)
	{
	}
	virtual const NotchExposedProperty::PropertyType GetPropertyType() const { return PropertyType_Texture; }
	virtual const NotchExposedProperty::PropertyDataType GetPropertyDataType() const { return PropertyDataType_Texture; }

	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_srv;
};

class NotchExposedPropertyFloat : public NotchExposedProperty
{
public:
	NotchExposedPropertyFloat()
		:
		m_propertyType(0), m_numChannels(0), m_min(0.0f), m_max(1.0f), m_dmxOffset(0), m_dmxIs16Bit(0), m_numArrayElements(0), m_floatValuesAllocated(NULL)
	{
		for (int i = 0; i < 16; ++i)
			m_floatValuesFixed[i] = 0;
	}
	virtual const NotchExposedProperty::PropertyType GetPropertyType() const { return (NotchExposedProperty::PropertyType)m_propertyType; }
	virtual const NotchExposedProperty::PropertyDataType GetPropertyDataType() const { return PropertyDataType_Float; }

	inline float* GetFloatValues() { return m_floatValuesAllocated ? m_floatValuesAllocated : m_floatValuesFixed; }
	inline const float* GetFloatValues() const { return m_floatValuesAllocated ? m_floatValuesAllocated : m_floatValuesFixed; }

	int m_propertyType;
	int m_numChannels;
	float m_min;
	float m_max;
	int m_dmxOffset;
	int m_dmxIs16Bit;
	int m_numArrayElements;
	float m_floatValuesFixed[16];
	float* m_floatValuesAllocated;
};

class NotchExposedPropertyInt : public NotchExposedProperty
{
public:
	NotchExposedPropertyInt()
		: m_propertyType(0)
		, m_intValue(0)
	{
	}

	virtual const NotchExposedProperty::PropertyType GetPropertyType() const { return (NotchExposedProperty::PropertyType)m_propertyType; }
	virtual const NotchExposedProperty::PropertyDataType GetPropertyDataType() const { return PropertyDataType_Int; }

	int m_propertyType;
	int m_intValue;
	std::vector< std::string > m_enums;
};

class NotchInstance
{
public:
	class Layer
	{
	public:
		std::string m_guid;
		std::string m_name;
		int m_index;
		std::vector< NotchExposedProperty* > m_properties;
	};

	NotchInstance(NotchBlock& m_notchBlock, int instanceId);
	~NotchInstance();
		
	bool SetLayer(NotchInstance::Layer* layer);
	bool SetTime(double currentTime, double timeDelta);
	bool Render(ID3D11Texture2D* targetTexture, ID3D11RenderTargetView* targetRTV);
	void Reset();
	void Deactivate();

	bool SetStringProperty(NotchExposedProperty* prop, const char* value);
	bool SetUnicodeStringProperty(NotchExposedProperty* prop, const wchar_t* value);
	bool SetTextureProperty(NotchExposedProperty* prop, ID3D11Texture2D* texture, ID3D11ShaderResourceView* srv);
	bool SetFloatProperty(NotchExposedProperty* prop, float value);
	bool SetFloatProperty(NotchExposedProperty* prop, float* values, int valueCount);
	bool SetIntProperty(NotchExposedProperty* prop, int value);
	void UpdateReadableProperty(NotchExposedProperty* prop);
	bool GetCurrentLayerDuration(double *duration);
	ColourSpace GetCurrentLayerWorkingColourSpace();

	inline NotchExposedPropertyInt* GetLayerSelectionProperty() { return m_layerSelectionProperty; }
	inline const std::vector< NotchExposedProperty* >& GetActiveProperties() const { return m_currentLayer ? m_currentLayer->m_properties : m_properties; }
	inline const std::vector< NotchExposedProperty* >& GetAllProperties() const { return m_properties; }
	inline const std::vector< NotchInstance::Layer* >& GetLayers() const { return m_layers; }
	inline const NotchBlock& GetNotchBlock() const { return m_notchBlock; }

	inline bool GetPropertyMapChanged() const { return m_propertyMapChanged; }

protected:
	bool SetCurrentLayer(const char* layerGUID);
	bool Init();
	bool InitProperties();
	bool InitLayers();

protected:
	Layer* m_currentLayer;

	std::vector< Layer* > m_layers;
	std::vector< NotchExposedProperty* > m_properties;
	NotchExposedPropertyInt* m_layerSelectionProperty;

	int m_instanceId;
	NotchBlock& m_notchBlock;

	bool m_propertyMapChanged;
};

class NotchBlock
{
	friend class NotchInstance;

public:
	NotchBlock();
	~NotchBlock();

	bool LoadBlock(const char* filename, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, 
		CLoggingCallback logCallback, std::vector<const char*>* commandLineArguments = NULL);
	NotchInstance* CreateInstance();
	bool ReleaseInstance(NotchInstance* inst);
	void PurgeVRAM();
	unsigned __int64 GetVRAMUsage();
	std::string GetCreatorsString();
	std::string GetVersionString();
	double GetDuration();
	bool SetInputOutputColourSpace(ColourSpace colourSpace);
	bool IsLayersAsSeparateEffectsEnabled(int *isEnabled);

	inline size_t GetNumInstances() const { return m_instances.size(); }
	inline const std::string& GetFilename() const { return m_filename; }

	ColourSpace m_colourSpace;
	ColourSpace m_projectColourSpace;

protected:
	bool LoadDLL(const char* filename);

protected:
	std::string m_filename;
	HMODULE m_pluginDll;
	std::vector< NotchInstance* > m_instances;

protected:
	_DemolitionCreate m_demolitionCreate;
	_DemolitionRelease m_demolitionRelease;
	_DemolitionInitRenderDeviceD3D11 m_demolitionInitRenderDeviceD3D11;
	_DemolitionLoadDFX m_demolitionLoadDFX;
	_DemolitionInitialiseDFX m_demolitionInitialiseDFX;
	_DemolitionReleaseDFXInstance m_demolitionReleaseDFXInstance;
	_DemolitionDeactivateDFX m_demolitionDeactivateDFX;
	_DemolitionPurgeVRAMPool m_demolitionPurgeVRAMPool;
	_DemolitionSetCurrentDocumentHandle m_demolitionSetCurrentDocumentHandle;
	_DemolitionGetCurrentDocumentHandle m_demolitionGetCurrentDocumentHandle;
	_DemolitionResetState m_demolitionResetState;
	_DemolitionSetTime m_demolitionSetTime;
	_DemolitionRenderFrameToRenderTarget m_demolitionRenderFrameToRenderTarget;
	_DemolitionGetLayerGUID m_demolitionGetLayerGUID;
	_DemolitionGetCurrentLayerGUID m_demolitionGetCurrentLayerGUID;
	_DemolitionGetLayerIndexByGUID m_demolitionGetLayerIndexByGUID;
	_DemolitionGetLayerDuration m_demolitionGetLayerDuration;
	_DemolitionGetBlockDuration m_demolitionGetBlockDuration;
	_DemolitionGetNumProperties m_demolitionGetNumProperties;
	_DemolitionGetPropertyUniqueIdByIndex m_demolitionGetPropertyUniqueIdByIndex;
	_DemolitionGetPropertyNameById m_demolitionGetPropertyNameById;
	_DemolitionGetPropertyGroupNameById m_demolitionGetPropertyGroupNameById;
	_DemolitionGetPropertyTypeById m_demolitionGetPropertyTypeById;
	_DemolitionGetPropertyEnumsById m_demolitionGetPropertyEnumsById;
	_DemolitionGetPropertyValueById m_demolitionGetPropertyValueById;
	_DemolitionSetPropertyValueById m_demolitionSetPropertyValueById;
	_DemolitionGetPropertyRangeById m_demolitionGetPropertyRangeById;
	_DemolitionSetUnicodePropertyValueById m_demolitionSetUnicodePropertyValueById;
	_DemolitionSetUnicodePropertyValueByIndex m_demolitionSetUnicodePropertyValueByIndex;
	_DemolitionSetPropertyDX11TextureSRVValueById m_demolitionSetPropertyDX11TextureSRVValueById;
	_DemolitionIsPropertyActiveInLayer m_demolitionIsPropertyActiveInLayer;
	_DemolitionSetFloatPropertyValueByIndex m_demolitionSetFloatPropertyValueByIndex;
	_DemolitionSetFloatPropertyValueById m_demolitionSetFloatPropertyValueById;
	_DemolitionGetPropertyDMXOffset m_demolitionGetPropertyDMXOffset;
	_DemolitionGetPropertyDMX16Bit m_demolitionGetPropertyDMX16Bit;
	_DemolitionGetPropertyReadable m_demolitionGetPropertyReadable;
	_DemolitionGetUnicodeStringPropertyValueById m_demolitionGetUnicodeStringPropertyValueById;
	_DemolitionGetUnicodeStringPropertyValueByIndex m_demolitionGetUnicodeStringPropertyValueByIndex;
	_DemolitionGetPropertyArrayElementCount m_demolitionGetPropertyArrayElementCount;
	_DemolitionSetFloatArrayPropertyValueByIndex m_demolitionSetFloatArrayPropertyValueByIndex;
	_DemolitionSetFloatArrayPropertyValueById m_demolitionSetFloatArrayPropertyValueById;
	_DemolitionGetFloatArrayPropertyValueById m_demolitionGetFloatArrayPropertyValueById;
	_DemolitionGetProjectWorkingColourSpace m_demolitionGetProjectWorkingColourSpace;
	_DemolitionGetLayerWorkingColourSpace m_demolitionGetLayerWorkingColourSpace;
	_DemolitionSetInputOutputColourSpace m_demolitionSetInputOutputColourSpace;
	_DemolitionIsLayersAsSeparateEffectsEnabled m_demolitionIsLayersAsSeparateEffectsEnabled;

	_DemolitionGetVersion m_demolitionGetVersion;
	_DemolitionGetVRAMUsage m_demolitionGetVRAMUsage;
	_GetCreatorsString m_getCreatorsString;
	_DemolitionHasExposedPropertiesChanged m_demolitionHasExposedPropertiesChanged;
	_DemolitionSetLogCallbackC m_demolitionSetLogCallbackC;
};

#endif
