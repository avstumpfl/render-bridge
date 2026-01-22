
#include "NotchBlock.h"
#include "common/string.h"
#include <stdlib.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef ERROR

NotchBlock::NotchBlock()
{
	m_pluginDll = NULL;
	m_colourSpace = ColourSpace_SRGBGamma22;
	m_projectColourSpace = ColourSpace_SRGBGamma22;

	m_demolitionCreate = NULL;
	m_demolitionRelease = NULL;
	m_demolitionInitRenderDeviceD3D11 = NULL;
	m_demolitionLoadDFX = NULL;
	m_demolitionInitialiseDFX = NULL;
	m_demolitionReleaseDFXInstance = NULL;
	m_demolitionDeactivateDFX = NULL;
	m_demolitionPurgeVRAMPool = NULL;
	m_demolitionSetCurrentDocumentHandle = NULL;
	m_demolitionGetCurrentDocumentHandle = NULL;
	m_demolitionResetState = NULL;
	m_demolitionSetTime = NULL;
	m_demolitionRenderFrameToRenderTarget = NULL;
	m_demolitionGetLayerGUID = NULL;
	m_demolitionGetCurrentLayerGUID = NULL;
	m_demolitionGetLayerIndexByGUID = NULL;
	m_demolitionGetLayerDuration = NULL;
	m_demolitionGetBlockDuration = NULL;
	m_demolitionGetNumProperties = NULL;
	m_demolitionGetPropertyUniqueIdByIndex = NULL;
	m_demolitionGetPropertyNameById = NULL;
	m_demolitionGetPropertyGroupNameById = NULL;
	m_demolitionGetPropertyTypeById = NULL;
	m_demolitionGetPropertyEnumsById = NULL;
	m_demolitionGetPropertyValueById = NULL;
	m_demolitionSetPropertyValueById = NULL;
	m_demolitionGetPropertyRangeById = NULL;
	m_demolitionSetUnicodePropertyValueById = NULL;
	m_demolitionSetUnicodePropertyValueByIndex = NULL;
	m_demolitionSetPropertyDX11TextureSRVValueById = NULL;
	m_demolitionIsPropertyActiveInLayer = NULL;
	m_demolitionSetFloatPropertyValueByIndex = NULL;
	m_demolitionSetFloatPropertyValueById = NULL;
	m_demolitionGetPropertyDMXOffset = NULL;
	m_demolitionGetPropertyDMX16Bit = NULL;
	m_demolitionGetPropertyReadable = NULL;
	m_demolitionGetVersion = NULL;
	m_demolitionGetVRAMUsage = NULL;
	m_getCreatorsString = NULL;
	m_demolitionHasExposedPropertiesChanged = NULL;
	m_demolitionSetLogCallbackC = NULL;
	m_demolitionGetUnicodeStringPropertyValueById = NULL;
	m_demolitionGetUnicodeStringPropertyValueByIndex = NULL;
	m_demolitionGetPropertyArrayElementCount = NULL;
	m_demolitionSetFloatArrayPropertyValueByIndex = NULL;
	m_demolitionSetFloatArrayPropertyValueById = NULL;
	m_demolitionGetFloatArrayPropertyValueById = NULL;
	m_demolitionGetProjectWorkingColourSpace = NULL;
	m_demolitionGetLayerWorkingColourSpace = NULL;
	m_demolitionSetInputOutputColourSpace = NULL;
	m_demolitionIsLayersAsSeparateEffectsEnabled = NULL;
}
NotchBlock::~NotchBlock()
{
	if(m_demolitionRelease)
	{
		m_demolitionRelease();
	}

	if (m_pluginDll) 
	{
		FreeLibrary(m_pluginDll);
	}
}

void NotchBlock::PurgeVRAM()
{
	if(m_demolitionPurgeVRAMPool)
	{
		m_demolitionPurgeVRAMPool();
	}
}
unsigned __int64 NotchBlock::GetVRAMUsage()
{
	if(m_demolitionGetVRAMUsage)
	{
		return m_demolitionGetVRAMUsage();
	}
	return 0;
}

std::string NotchBlock::GetCreatorsString()
{
	if(m_getCreatorsString)
	{
		char rsltBuffer[1024] = {NULL};
		m_getCreatorsString(rsltBuffer, 1024);

		return std::string(rsltBuffer);
	}
	return "";
}

std::string NotchBlock::GetVersionString()
{
	if(m_demolitionGetVersion)
	{
		unsigned int version = m_demolitionGetVersion();

		char rsltBuffer[1024] = {NULL};
		sprintf_s(rsltBuffer, 1024, "%d.%d.%d.%03d",
		(version >> 20) & 0xff,
		(version >> 16) & 0xf,
		(version >> 8) & 0xff,
		(version >> 0) & 0xff
		);
		
		return std::string(rsltBuffer);
	}
	return "";
}

double NotchBlock::GetDuration()
{
	double duration = 0.0;
	if (m_demolitionGetBlockDuration)
	{
		m_demolitionGetBlockDuration(&duration);
	}
	return duration;
}

bool NotchBlock::SetInputOutputColourSpace(ColourSpace colourSpace)
{
	if (m_demolitionSetInputOutputColourSpace)
	{
		m_demolitionSetInputOutputColourSpace(colourSpace);
		return true;
	}
	return false;
}

bool NotchBlock::IsLayersAsSeparateEffectsEnabled(int *isEnabled)
{
	if (m_demolitionIsLayersAsSeparateEffectsEnabled)
	{
		*isEnabled = m_demolitionIsLayersAsSeparateEffectsEnabled();
		return true;
	}
	return false;
}

bool NotchBlock::LoadBlock(const char* filename, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext, 
	CLoggingCallback logCallback, std::vector<const char*>* commandLineArguments)
{
	m_filename = filename;

	bool dllrslt = LoadDLL(filename);
	if(!dllrslt)
		return false;

	std::string args = "+document test.dfx +nopassword +exposecoloursascolours 1 ";
#if !defined(NDEBUG)
	args += "+debug 1 ";
#endif
	if(commandLineArguments)
	{
		for(size_t i = 0; i < commandLineArguments->size(); ++i)
		{
			args += (*commandLineArguments)[i];
			args += " ";
		}
	}

	// create demolition
	int rslt = m_demolitionCreate(args.c_str());
	if(!rslt)
		return false;

	if(m_demolitionSetLogCallbackC)
	{
		m_demolitionSetLogCallbackC(logCallback);
	}

	// load the DFX 
	rslt = m_demolitionLoadDFX(filename);
	if(!rslt)
		return false;

	// init the render device
	rslt = m_demolitionInitRenderDeviceD3D11(d3dDevice, d3dDeviceContext);
	if(!rslt)
		return false;

	// retrieve the project working colour space
	if(m_demolitionGetProjectWorkingColourSpace)
		m_projectColourSpace = (ColourSpace)m_demolitionGetProjectWorkingColourSpace();

	return true;
}

NotchInstance* NotchBlock::CreateInstance()
{	
	if(m_demolitionInitialiseDFX == NULL)
		return NULL;

	int docHandle = m_demolitionInitialiseDFX();
	if(!docHandle)
		return NULL;

	NotchInstance* inst = new NotchInstance(*this, docHandle);
	m_instances.push_back(inst);

	return inst;
}

bool NotchBlock::ReleaseInstance(NotchInstance* inst)
{
	// find instance
	if(inst)
	{
		for(size_t i = 0; i < m_instances.size(); ++i)
		{
			if(m_instances[i] == inst)
			{
				m_instances.erase(m_instances.begin() + i);
				delete inst;
				return true;
			}
		}
	}
	
	return false;
}

bool NotchBlock::LoadDLL(const char* filename)
{
	m_pluginDll = LoadLibraryW(common::utf8_to_wide(filename).c_str());
	if (m_pluginDll == NULL)
	{
		return false;
	}
	
	m_demolitionCreate = (_DemolitionCreate)GetProcAddress(m_pluginDll, "DemolitionCreate");
	m_demolitionRelease = (_DemolitionRelease)GetProcAddress(m_pluginDll, "DemolitionRelease");
	m_demolitionInitRenderDeviceD3D11 = (_DemolitionInitRenderDeviceD3D11)GetProcAddress(m_pluginDll, "DemolitionInitRenderDeviceD3D11");
	m_demolitionLoadDFX = (_DemolitionLoadDFX)GetProcAddress(m_pluginDll, "DemolitionLoadDFX");
	m_demolitionInitialiseDFX = (_DemolitionInitialiseDFX)GetProcAddress(m_pluginDll, "DemolitionInitialiseDFX");
	m_demolitionReleaseDFXInstance = (_DemolitionReleaseDFXInstance)GetProcAddress(m_pluginDll, "DemolitionReleaseDFXInstance");
	m_demolitionDeactivateDFX = (_DemolitionDeactivateDFX)GetProcAddress(m_pluginDll, "DemolitionDeactivateDFX");
	m_demolitionPurgeVRAMPool = (_DemolitionPurgeVRAMPool)GetProcAddress(m_pluginDll, "DemolitionPurgeVRAMPool");
	m_demolitionSetCurrentDocumentHandle = (_DemolitionSetCurrentDocumentHandle)GetProcAddress(m_pluginDll, "DemolitionSetCurrentDocumentHandle");
	m_demolitionGetCurrentDocumentHandle = (_DemolitionGetCurrentDocumentHandle)GetProcAddress(m_pluginDll, "DemolitionGetCurrentDocumentHandle");
	m_demolitionResetState = (_DemolitionResetState)GetProcAddress(m_pluginDll, "DemolitionResetState");
	m_demolitionSetTime = (_DemolitionSetTime)GetProcAddress(m_pluginDll, "DemolitionSetTime");
	m_demolitionRenderFrameToRenderTarget = (_DemolitionRenderFrameToRenderTarget)GetProcAddress(m_pluginDll, "DemolitionRenderFrameToRenderTarget");
	m_demolitionGetCurrentLayerGUID = (_DemolitionGetCurrentLayerGUID)GetProcAddress(m_pluginDll, "DemolitionGetCurrentLayerGUID");
	m_demolitionGetLayerGUID = (_DemolitionGetLayerGUID)GetProcAddress(m_pluginDll, "DemolitionGetLayerGUID");
	m_demolitionGetLayerIndexByGUID = (_DemolitionGetLayerIndexByGUID)GetProcAddress(m_pluginDll, "DemolitionGetLayerIndexByGUID");
	m_demolitionGetLayerDuration = (_DemolitionGetLayerDuration)GetProcAddress(m_pluginDll, "DemolitionGetLayerDuration");
	m_demolitionGetBlockDuration = (_DemolitionGetBlockDuration)GetProcAddress(m_pluginDll, "DemolitionGetBlockDuration");
	m_demolitionGetNumProperties = (_DemolitionGetNumProperties)GetProcAddress(m_pluginDll, "DemolitionGetNumProperties");
	m_demolitionGetPropertyUniqueIdByIndex = (_DemolitionGetPropertyUniqueIdByIndex)GetProcAddress(m_pluginDll, "DemolitionGetPropertyUniqueIdByIndex");
	m_demolitionGetPropertyNameById =(_DemolitionGetPropertyNameById)GetProcAddress(m_pluginDll, "DemolitionGetPropertyNameById");
	m_demolitionGetPropertyGroupNameById =(_DemolitionGetPropertyGroupNameById)GetProcAddress(m_pluginDll, "DemolitionGetPropertyGroupNameById");
	m_demolitionGetPropertyTypeById = (_DemolitionGetPropertyTypeById)GetProcAddress(m_pluginDll, "DemolitionGetPropertyTypeById");
	m_demolitionGetPropertyEnumsById = (_DemolitionGetPropertyEnumsById)GetProcAddress(m_pluginDll, "DemolitionGetPropertyEnumsById");
	m_demolitionGetPropertyValueById = (_DemolitionGetPropertyValueById)GetProcAddress(m_pluginDll, "DemolitionGetPropertyValueById");
	m_demolitionSetPropertyValueById = (_DemolitionSetPropertyValueById)GetProcAddress(m_pluginDll, "DemolitionSetPropertyValueById");
	m_demolitionGetPropertyRangeById = (_DemolitionGetPropertyRangeById)GetProcAddress(m_pluginDll, "DemolitionGetPropertyRangeById");
	m_demolitionSetPropertyDX11TextureSRVValueById =(_DemolitionSetPropertyDX11TextureSRVValueById)GetProcAddress(m_pluginDll, "DemolitionSetPropertyDX11TextureSRVValueById");
	m_demolitionIsPropertyActiveInLayer =(_DemolitionIsPropertyActiveInLayer)GetProcAddress(m_pluginDll, "DemolitionIsPropertyActiveInLayer");
	m_demolitionSetFloatPropertyValueByIndex = (_DemolitionSetFloatPropertyValueByIndex)GetProcAddress(m_pluginDll, "DemolitionSetFloatPropertyValueByIndex");
	m_demolitionSetFloatPropertyValueById = (_DemolitionSetFloatPropertyValueById)GetProcAddress(m_pluginDll, "DemolitionSetFloatPropertyValueById");
	m_demolitionGetPropertyDMXOffset = (_DemolitionGetPropertyDMXOffset)GetProcAddress(m_pluginDll, "DemolitionGetPropertyDMXOffset");
	m_demolitionGetPropertyDMX16Bit = (_DemolitionGetPropertyDMX16Bit)GetProcAddress(m_pluginDll, "DemolitionGetPropertyDMX16Bit");
	m_demolitionGetPropertyReadable = (_DemolitionGetPropertyReadable)GetProcAddress(m_pluginDll, "DemolitionGetPropertyReadable");
	m_demolitionGetVersion = (_DemolitionGetVersion)GetProcAddress(m_pluginDll, "DemolitionGetVersion");
	m_demolitionGetVRAMUsage = (_DemolitionGetVRAMUsage)GetProcAddress(m_pluginDll, "DemolitionGetVRAMUsage");
	m_demolitionHasExposedPropertiesChanged = (_DemolitionHasExposedPropertiesChanged)GetProcAddress(m_pluginDll, "DemolitionHasExposedPropertiesChanged");
	m_getCreatorsString = (_GetCreatorsString)GetProcAddress(m_pluginDll, "GetCreatorsString");
	m_demolitionSetUnicodePropertyValueById = (_DemolitionSetUnicodePropertyValueById)GetProcAddress(m_pluginDll, "DemolitionSetUnicodePropertyValueById");
	m_demolitionSetUnicodePropertyValueByIndex = (_DemolitionSetUnicodePropertyValueByIndex)GetProcAddress(m_pluginDll, "DemolitionSetUnicodePropertyValueByIndex");
	m_demolitionSetLogCallbackC = (_DemolitionSetLogCallbackC)GetProcAddress(m_pluginDll, "DemolitionSetLogCallbackC");
	m_demolitionGetUnicodeStringPropertyValueById = (_DemolitionGetUnicodeStringPropertyValueById)GetProcAddress(m_pluginDll, "DemolitionGetUnicodeStringPropertyValueById");
	m_demolitionGetUnicodeStringPropertyValueByIndex = (_DemolitionGetUnicodeStringPropertyValueByIndex)GetProcAddress(m_pluginDll, "DemolitionGetUnicodeStringPropertyValueByIndex");
	m_demolitionGetPropertyArrayElementCount = (_DemolitionGetPropertyArrayElementCount)GetProcAddress(m_pluginDll, "DemolitionGetPropertyArrayElementCount");
	m_demolitionSetFloatArrayPropertyValueByIndex = (_DemolitionSetFloatArrayPropertyValueByIndex)GetProcAddress(m_pluginDll, "DemolitionSetFloatArrayPropertyValueByIndex");
	m_demolitionSetFloatArrayPropertyValueById = (_DemolitionSetFloatArrayPropertyValueById)GetProcAddress(m_pluginDll, "DemolitionSetFloatArrayPropertyValueById");
	m_demolitionGetFloatArrayPropertyValueById = (_DemolitionGetFloatArrayPropertyValueById)GetProcAddress(m_pluginDll, "DemolitionGetFloatArrayPropertyValueById");
	m_demolitionGetProjectWorkingColourSpace = (_DemolitionGetProjectWorkingColourSpace)GetProcAddress(m_pluginDll, "DemolitionGetProjectWorkingColourSpace");
	m_demolitionGetLayerWorkingColourSpace = (_DemolitionGetLayerWorkingColourSpace)GetProcAddress(m_pluginDll, "DemolitionGetLayerWorkingColourSpace");
	m_demolitionSetInputOutputColourSpace = (_DemolitionSetInputOutputColourSpace)GetProcAddress(m_pluginDll, "DemolitionSetInputOutputColourSpace");
	m_demolitionIsLayersAsSeparateEffectsEnabled = (_DemolitionIsLayersAsSeparateEffectsEnabled)GetProcAddress(m_pluginDll, "DemolitionIsLayersAsSeparateEffectsEnabled");
	
	return true;
}





NotchInstance::NotchInstance(NotchBlock& notchBlock, int instanceId)
	:
 	m_currentLayer(NULL)
,	m_layerSelectionProperty(NULL)
,	m_instanceId(instanceId)
,	m_notchBlock(notchBlock)
,	m_propertyMapChanged(false)
{
	Init();
}
NotchInstance::~NotchInstance()
{
	for(size_t i = 0; i < m_properties.size(); ++i)
	{
		NotchExposedProperty* prop = m_properties[i];
		if(prop)
		{
			delete prop;
		}
	}
	m_properties.clear();

	for(size_t i = 0; i < m_layers.size(); ++i)
	{
		Layer* layer = m_layers[i];
		if(layer)
		{
			delete layer;
		}
	}
	m_layers.clear();

	if (m_layerSelectionProperty)
	{
		delete m_layerSelectionProperty;
		m_layerSelectionProperty = NULL;
	}
}

void NotchInstance::Reset()
{
	if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
	{
		m_notchBlock.m_demolitionSetCurrentDocumentHandle(m_instanceId);
	}

	
	// reset all properties to initial values
	for(size_t i = 0; i < m_properties.size(); ++i)
	{
		m_notchBlock.m_demolitionSetPropertyValueById(m_properties[i]->m_initialValue.c_str(), m_properties[i]->m_propertyId.c_str());
	}

	// select first layer
	if(m_layers.size())
	{
		SetCurrentLayer(m_layers[0]->m_guid.c_str());
	}

	// reset
	if(m_notchBlock.m_demolitionResetState)
	{
		m_notchBlock.m_demolitionResetState();
	}
}

bool NotchInstance::SetLayer(NotchInstance::Layer* layer)
{
	if(layer == m_currentLayer)
		return true;

	if(layer)
	{
		return SetCurrentLayer(layer->m_guid.c_str());
	}
	return false;
}

bool NotchInstance::SetCurrentLayer(const char* layerGUID)
{
	m_currentLayer = NULL;

	if (m_layerSelectionProperty)
	{
		if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
		{
			m_notchBlock.m_demolitionSetCurrentDocumentHandle(m_instanceId);
		}
		for (size_t i = 0; i < m_layers.size(); ++i)
		{
			if (m_layers[i]->m_guid == layerGUID)
			{
				m_currentLayer = m_layers[i];
			}
		}
		if(m_currentLayer)
		{
			SetIntProperty(m_layerSelectionProperty, m_currentLayer->m_index);
		}
	}
	return true;
}

bool NotchInstance::GetCurrentLayerDuration(double *duration)
{
	if (m_currentLayer && m_notchBlock.m_demolitionGetLayerDuration)
	{
		int rslt = m_notchBlock.m_demolitionGetLayerDuration(duration, m_currentLayer->m_guid.c_str());
		return rslt != 0; 
	}

	return false;
}

void NotchInstance::UpdateReadableProperty(NotchExposedProperty* prop)
{
	if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
	{
		m_notchBlock.m_demolitionSetCurrentDocumentHandle(m_instanceId);
	}
	if(m_notchBlock.m_demolitionGetPropertyValueById)
	{
		char propertyValue[1024] = {0};
		if(m_notchBlock.m_demolitionGetPropertyValueById(propertyValue, prop->m_propertyId.c_str()) != 0)
		{
			prop->m_initialValue = propertyValue;
		}
	}
}

bool NotchInstance::SetStringProperty(NotchExposedProperty* prop, const char* value)
{
	if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
	{
		m_notchBlock.m_demolitionSetCurrentDocumentHandle(m_instanceId);
	}
	if(m_notchBlock.m_demolitionSetPropertyValueById)
	{
		int rslt = m_notchBlock.m_demolitionSetPropertyValueById(value, prop->m_propertyId.c_str());
		return rslt != 0;
	}
	return false;
}

bool NotchInstance::SetUnicodeStringProperty(NotchExposedProperty* prop, const wchar_t* value)
{
	if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
	{
		m_notchBlock.m_demolitionSetCurrentDocumentHandle(m_instanceId);
	}
	if(m_notchBlock.m_demolitionSetUnicodePropertyValueById)
	{
		int rslt = m_notchBlock.m_demolitionSetUnicodePropertyValueById(value, prop->m_propertyId.c_str());
		return rslt != 0;
	}
	return false;
}
bool NotchInstance::SetTextureProperty(NotchExposedProperty* prop, ID3D11Texture2D* texture, ID3D11ShaderResourceView* srv)
{
	if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
	{
		m_notchBlock.m_demolitionSetCurrentDocumentHandle(m_instanceId);
	}
	if(m_notchBlock.m_demolitionSetPropertyDX11TextureSRVValueById)
	{
		int rslt = m_notchBlock.m_demolitionSetPropertyDX11TextureSRVValueById(texture, srv, prop->m_propertyId.c_str());
		return rslt != 0;
	}
	return false;
}
bool NotchInstance::SetFloatProperty(NotchExposedProperty* prop, float value)
{
	if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
	{
		m_notchBlock.m_demolitionSetCurrentDocumentHandle(m_instanceId);
	}
	if(m_notchBlock.m_demolitionSetFloatPropertyValueByIndex && prop->GetPropertyType() == NotchExposedProperty::PropertyType_Float && prop->m_index != 0xffffffff)
	{
		int rslt = m_notchBlock.m_demolitionSetFloatPropertyValueByIndex(value, prop->m_index);
		return rslt != 0;
	}
	else if(m_notchBlock.m_demolitionSetFloatPropertyValueById && prop->GetPropertyType() == NotchExposedProperty::PropertyType_Float)
	{
		int rslt = m_notchBlock.m_demolitionSetFloatPropertyValueById(value, prop->m_propertyId.c_str());
		return rslt != 0;
	}
	else
	{
		char propertyValue[64];
		sprintf_s(propertyValue, "%.4f", value);
		int rslt = m_notchBlock.m_demolitionSetPropertyValueById(propertyValue, prop->m_propertyId.c_str());
		return rslt != 0;
	}
	return false;
}
bool NotchInstance::SetFloatProperty(NotchExposedProperty* prop, float* values, int valueCount)
{
	if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
	{
		m_notchBlock.m_demolitionSetCurrentDocumentHandle(m_instanceId);
	}
	if(m_notchBlock.m_demolitionSetFloatArrayPropertyValueByIndex && prop->GetPropertyDataType() == NotchExposedProperty::PropertyType_Float && prop->m_index != 0xffffffff && ((NotchExposedPropertyFloat*)prop)->m_numArrayElements > 0)
	{
		int rslt = m_notchBlock.m_demolitionSetFloatArrayPropertyValueByIndex(values, prop->m_index, valueCount);
		return rslt != 0;
	}
	else if(m_notchBlock.m_demolitionSetFloatArrayPropertyValueById && prop->GetPropertyDataType() == NotchExposedProperty::PropertyType_Float && ((NotchExposedPropertyFloat*)prop)->m_numArrayElements > 0)
	{
		int rslt = m_notchBlock.m_demolitionSetFloatArrayPropertyValueById(values, prop->m_propertyId.c_str(), valueCount);
		return rslt != 0;
	}
	else if(m_notchBlock.m_demolitionSetFloatPropertyValueByIndex && prop->GetPropertyType() == NotchExposedProperty::PropertyType_Float && valueCount == 1 && prop->m_index != 0xffffffff)
	{
		int rslt = m_notchBlock.m_demolitionSetFloatPropertyValueByIndex(values[0], prop->m_index);
		return rslt != 0;
	}
	else if(m_notchBlock.m_demolitionSetFloatPropertyValueById && prop->GetPropertyType() == NotchExposedProperty::PropertyType_Float && valueCount == 1)
	{
		int rslt = m_notchBlock.m_demolitionSetFloatPropertyValueById(values[0], prop->m_propertyId.c_str());
		return rslt != 0;
	}
	else
	{
		if (m_notchBlock.m_demolitionSetPropertyValueById)
		{
			char propertyValue[256];

			if (prop->GetPropertyType() == NotchExposedProperty::PropertyType_Float)
			{
				valueCount = min(valueCount, ((NotchExposedPropertyFloat*)prop)->m_numChannels);
			}

			switch (valueCount)
			{
			case 13:
			{
				sprintf_s(propertyValue, "%.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f", values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7], values[8], values[9], values[10], values[11], values[12]);
			}
			break;
			case 7:
			{
				sprintf_s(propertyValue, "%.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f", values[0], values[1], values[2], values[3], values[4], values[5], values[6]);
			}
			break;
			case 4:
			{
				sprintf_s(propertyValue, "%.4f, %.4f, %.4f, %.4f", values[0], values[1], values[2], values[3]);
			}
			break;
			case 3:
			{
				sprintf_s(propertyValue, "%.4f, %.4f, %.4f", values[0], values[1], values[2]);
			}
			break;
			case 2:
			{
				sprintf_s(propertyValue, "%.4f, %.4f", values[0], values[1]);
			}
			break;
			default:
			{
				sprintf_s(propertyValue, "%.4f", values[0]);
			}
			break;
			}
			int rslt = m_notchBlock.m_demolitionSetPropertyValueById(propertyValue, prop->m_propertyId.c_str());
			return rslt != 0;
		}
	}
	return false;
}
bool NotchInstance::SetIntProperty(NotchExposedProperty* prop, int value)
{
	if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
	{
		m_notchBlock.m_demolitionSetCurrentDocumentHandle(m_instanceId);
	}
	if (m_notchBlock.m_demolitionSetPropertyValueById)
	{
		char propertyValue[64];
		sprintf_s(propertyValue, "%d", value);

		int rslt = m_notchBlock.m_demolitionSetPropertyValueById(propertyValue, prop->m_propertyId.c_str());
		return rslt != 0;
	}
	return false;
}

bool NotchInstance::SetTime(double currentTime, double timeDelta)
{
	if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
	{
		m_notchBlock.m_demolitionSetCurrentDocumentHandle(m_instanceId);
	}
	if(m_notchBlock.m_demolitionSetTime)
	{
		int rslt = m_notchBlock.m_demolitionSetTime(currentTime, timeDelta);
		return rslt != 0;
	}
	return false;
}

ColourSpace NotchInstance::GetCurrentLayerWorkingColourSpace()
{
	ColourSpace colourSpace = ColourSpace_SRGBGamma22;

	int isLayersAsSeparateEffectsEnabled = 0;
	bool rslt = m_notchBlock.IsLayersAsSeparateEffectsEnabled(&isLayersAsSeparateEffectsEnabled);
	if(rslt && isLayersAsSeparateEffectsEnabled)
	{
		if (m_currentLayer && m_notchBlock.m_demolitionGetLayerWorkingColourSpace)
		{
			colourSpace = (ColourSpace)m_notchBlock.m_demolitionGetLayerWorkingColourSpace(m_currentLayer->m_guid.c_str());
		}
	}
	else
	{
		if (m_notchBlock.m_demolitionGetProjectWorkingColourSpace)
		{
			colourSpace = (ColourSpace)m_notchBlock.m_demolitionGetProjectWorkingColourSpace();
		}
	}
	return colourSpace;
}

bool NotchInstance::Render(ID3D11Texture2D* targetTexture, ID3D11RenderTargetView* targetRTV)
{
	m_propertyMapChanged = false;

	if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
	{
		m_notchBlock.m_demolitionSetCurrentDocumentHandle(m_instanceId);
	}
	// disabled property invalidation
	if(false)// m_notchBlock.m_demolitionHasExposedPropertiesChanged)
	{
		int havePropertiesChanged = m_notchBlock.m_demolitionHasExposedPropertiesChanged();
		if(havePropertiesChanged)
		{
			InitProperties();

			// you have to reinit the layers too - to update the active property lists.
			InitLayers();

			m_propertyMapChanged = true;
		}
	}

	if(m_notchBlock.m_demolitionRenderFrameToRenderTarget)
	{
		int rslt = m_notchBlock.m_demolitionRenderFrameToRenderTarget(targetTexture, targetRTV);
		return rslt != 0;
	}
	return false;
}
void NotchInstance::Deactivate()
{
	if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
	{
		m_notchBlock.m_demolitionSetCurrentDocumentHandle(m_instanceId);
	}
	if(m_notchBlock.m_demolitionDeactivateDFX)
	{
		m_notchBlock.m_demolitionDeactivateDFX();
	}
}
bool NotchInstance::Init()
{
	InitProperties();
	InitLayers();

	return true;

}

bool NotchInstance::InitProperties()
{


	int documentHandle = m_instanceId;
	if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
	{
		m_notchBlock.m_demolitionSetCurrentDocumentHandle(documentHandle);
	}

	if(m_notchBlock.m_demolitionGetNumProperties == NULL
	|| m_notchBlock.m_demolitionGetPropertyUniqueIdByIndex == NULL
	|| m_notchBlock.m_demolitionGetPropertyNameById == NULL
	|| m_notchBlock.m_demolitionGetPropertyGroupNameById == NULL
	|| m_notchBlock.m_demolitionGetPropertyTypeById == NULL
	|| m_notchBlock.m_demolitionGetPropertyValueById == NULL
	|| m_notchBlock.m_demolitionGetPropertyEnumsById == NULL
	|| m_notchBlock.m_demolitionGetPropertyRangeById == NULL)
	{
		return false;
	}

	
	for(size_t i = 0; i < m_properties.size(); ++i)
	{
		NotchExposedProperty* prop = m_properties[i];
		if(prop)
		{
			delete prop;
		}
	}
	m_properties.clear();
	if (m_layerSelectionProperty)
	{
		delete m_layerSelectionProperty;
		m_layerSelectionProperty = NULL;
	}
	

	int numProperties = m_notchBlock.m_demolitionGetNumProperties();

	for(int i = 0; i < numProperties; ++i)
	{
		char propertyId[1024];
		if(m_notchBlock.m_demolitionGetPropertyUniqueIdByIndex(propertyId, i))
		{
			// get name, value
			char propertyName[1024];
			char propertyGroupName[1024];
			char propertyType[1024];
			char propertyValue[1024];
			
			m_notchBlock.m_demolitionGetPropertyNameById(propertyName, propertyId);
			m_notchBlock.m_demolitionGetPropertyGroupNameById(propertyGroupName, propertyId);
			m_notchBlock.m_demolitionGetPropertyTypeById(propertyType, propertyId);
			
			int numArrayElements = 0;
			if (m_notchBlock.m_demolitionGetPropertyArrayElementCount)
			{
				numArrayElements = m_notchBlock.m_demolitionGetPropertyArrayElementCount(propertyId);
			}

			// only grab the property value for non-array properties.
			propertyValue[0] = 0;
			if(numArrayElements == 0)
			{
				m_notchBlock.m_demolitionGetPropertyValueById(propertyValue, propertyId);
			}

			int readable = 0;
			if(m_notchBlock.m_demolitionGetPropertyReadable)
			{
				readable = m_notchBlock.m_demolitionGetPropertyReadable(propertyId);
			}

			if(strcmp(propertyType,"image") == 0)
			{
				// an image
				NotchExposedPropertyImage* imageProperty = new NotchExposedPropertyImage;
				imageProperty->m_propertyId = propertyId;
				imageProperty->m_texture = NULL;
				imageProperty->m_srv = NULL;
				imageProperty->m_name = propertyName;
				imageProperty->m_groupName = propertyGroupName;
				imageProperty->m_index = i;
				imageProperty->m_initialValue = propertyValue;
				imageProperty->m_readable = readable; // they won't actually work as "readable" though.
				
				m_properties.push_back(imageProperty);
			}
			else if(strcmp(propertyType,"string") == 0)
			{
				// a string
				NotchExposedPropertyString* stringProperty = new NotchExposedPropertyString;
				stringProperty->m_propertyId = propertyId;
				stringProperty->m_value = propertyValue;
				stringProperty->m_name = propertyName;
				stringProperty->m_groupName = propertyGroupName;
				stringProperty->m_index = i;
				stringProperty->m_initialValue = propertyValue;
				stringProperty->m_readable = readable;

				// read the value as unicode, too, if possible.
				if (m_notchBlock.m_demolitionGetUnicodeStringPropertyValueById)
				{
					wchar_t unicodeValue[1024];
					if (m_notchBlock.m_demolitionGetUnicodeStringPropertyValueById(unicodeValue, propertyId))
					{
						stringProperty->m_unicodeValue = unicodeValue;
					}
				}

				strncpy_s(stringProperty->m_buffer, 1024, propertyValue, min(strlen(propertyValue), 1023));

				m_properties.push_back(stringProperty);
			}
			else if(strcmp(propertyType,"int1") == 0
				|| strcmp(propertyType,"int") == 0
				|| strcmp(propertyType,"enum") == 0
				|| strcmp(propertyType,"resource") == 0)
			{
				
				NotchExposedPropertyInt* numProperty = new NotchExposedPropertyInt;
				numProperty->m_propertyId = propertyId;
				numProperty->m_name = propertyName;
				numProperty->m_groupName = propertyGroupName;
				numProperty->m_index = i;
				numProperty->m_initialValue = propertyValue;
				numProperty->m_readable = readable;
				numProperty->m_intValue = atoi(propertyValue);

				char enumNames[4096];
				if (m_notchBlock.m_demolitionGetPropertyEnumsById(enumNames, propertyId))
				{
					// parse comma-separated enum list
					int enumStringPos = 0;
					int enumStringPosPrev = 0;
					int enumStringLength = (int)strlen(enumNames);

					char enumSubString[1024];

					while (enumStringPos < enumStringLength)
					{
						if (enumNames[enumStringPos] == ',' || (enumStringPos + 1 == enumStringLength))
						{
							int numCharacters = enumStringPos - enumStringPosPrev;
							if (enumStringPos + 1 == enumStringLength)
								++numCharacters;

							strncpy_s(enumSubString, &enumNames[enumStringPosPrev], numCharacters);
							enumSubString[numCharacters] = 0;

							numProperty->m_enums.push_back(enumSubString);

							// skip the space
							++enumStringPos;
							enumStringPosPrev = enumStringPos + 1;
						}
						++enumStringPos;
					}
				}

				if(strcmp(propertyType,"enum") == 0 && numProperty->m_enums.size() > 0)	
				{
					numProperty->m_propertyType = NotchExposedProperty::PropertyType_Enum;
				}
				else if(strcmp(propertyType,"resource") == 0)	
				{
					numProperty->m_propertyType = NotchExposedProperty::PropertyType_Resource;
				}
				else
				{
					numProperty->m_propertyType = NotchExposedProperty::PropertyType_Int;
				}


				if(strcmp(propertyId, "layerlayerlayer") == 0)
				{
					if(m_layerSelectionProperty == NULL)
					{
						m_layerSelectionProperty = numProperty;
					}
					else
					{
						delete numProperty; 
					}
				}
				else 
				{
					m_properties.push_back(numProperty);
				}
			}
			else if (strncmp(propertyType, "float", 5) == 0
				|| strncmp(propertyType, "colour", 6) == 0
				|| strncmp(propertyType, "posquat", 11) == 0
				|| strncmp(propertyType, "cameraparams", 13) == 0
				|| strncmp(propertyType, "posrotscale", 11) == 0
				|| strncmp(propertyType, "posquatscale", 12) == 0
				|| strncmp(propertyType, "matrix4x4", 9) == 0
				)
			{
				NotchExposedPropertyFloat* numProperty = new NotchExposedPropertyFloat;
				numProperty->m_propertyId = propertyId;
				numProperty->m_name = propertyName;
				numProperty->m_groupName = propertyGroupName;
				numProperty->m_index = i;
				numProperty->m_min = 0.0f;
				numProperty->m_max = 1.0f;
				numProperty->m_initialValue = propertyValue;
				numProperty->m_numChannels = 0;
				numProperty->m_dmxOffset = -1;
				numProperty->m_dmxIs16Bit = 0;
				numProperty->m_readable = readable;
				numProperty->m_numArrayElements = numArrayElements;

				int expectedValues = 1;
				if (strncmp(propertyType, "float2", 6) == 0)
				{
					numProperty->m_propertyType = NotchExposedProperty::PropertyType_Float;
					expectedValues = 2;
				}
				else if (strncmp(propertyType, "float3", 6) == 0)
				{
					numProperty->m_propertyType = NotchExposedProperty::PropertyType_Float;
					expectedValues = 3;
				}
				else if (strncmp(propertyType, "float4", 6) == 0)
				{
					numProperty->m_propertyType = NotchExposedProperty::PropertyType_Float;
					expectedValues = 4;
				}
				else if (strncmp(propertyType, "colour", 6) == 0)
				{
					numProperty->m_propertyType = NotchExposedProperty::PropertyType_Colour;
					expectedValues = 4;
				}
				else if (strncmp(propertyType, "posquat", 11) == 0)
				{
					numProperty->m_propertyType = NotchExposedProperty::PropertyType_ExposedNull;
					expectedValues = 7;
				}
				else if (strncmp(propertyType, "cameraparams", 13) == 0)
				{
					numProperty->m_propertyType = NotchExposedProperty::PropertyType_ExposedCamera;
					expectedValues = 13;
				}
				else if (strncmp(propertyType, "posrotscale", 13) == 0)
				{
					numProperty->m_propertyType = NotchExposedProperty::PropertyType_PosRotScale;
					expectedValues = 9;
				}
				else if (strncmp(propertyType, "posquatscale", 13) == 0)
				{
					numProperty->m_propertyType = NotchExposedProperty::PropertyType_PosRotQuatScale;
					expectedValues = 10;
				}
				else if (strncmp(propertyType, "matrix4x4", 13) == 0)
				{
					numProperty->m_propertyType = NotchExposedProperty::PropertyType_Matrix4x4;
					expectedValues = 16;
				}
				else
				{
					numProperty->m_propertyType = NotchExposedProperty::PropertyType_Float;
				}

				if (numProperty->m_numArrayElements)
				{
					expectedValues *= numProperty->m_numArrayElements;
				}

				if (expectedValues > 16)
				{
					numProperty->m_floatValuesAllocated = new float[expectedValues];
					memset(numProperty->m_floatValuesAllocated, 0, sizeof(float) * expectedValues);
				}

				if (m_notchBlock.m_demolitionGetFloatArrayPropertyValueById)
				{
					// the fast way. grab all the float data as an array.

					m_notchBlock.m_demolitionGetFloatArrayPropertyValueById(numProperty->GetFloatValues(), propertyId, expectedValues);
					numProperty->m_numChannels = expectedValues;
				}
				else
				{
					// the legacy way.
					// the channels are in a comma-separated string.
					// parse the number of channels in the value string, and extract the channel value from the value string.
					int valueStringPos = 0;
					int valueStringPosPrev = 0;
					int valueStringLength = (int)strlen(propertyValue);
					char valueSubString[1024];

					while (valueStringPos < valueStringLength)
					{
						if (propertyValue[valueStringPos] == ',' || (valueStringPos + 1 == valueStringLength))
						{
							int numCharacters = valueStringPos - valueStringPosPrev;
							if (valueStringPos + 1 == valueStringLength)
								++numCharacters;

							strncpy_s(valueSubString, &propertyValue[valueStringPosPrev], numCharacters);
							valueSubString[numCharacters] = 0;

							if (numProperty->m_numChannels < expectedValues)
							{
								numProperty->GetFloatValues()[numProperty->m_numChannels] = (float)atof(valueSubString);
								++numProperty->m_numChannels;
							}

							valueStringPosPrev = valueStringPos + 1;
						}
						++valueStringPos;
					}
				}
								

				if(m_notchBlock.m_demolitionGetPropertyRangeById(&numProperty->m_min, &numProperty->m_max, propertyId))
				{
					// has range set (use supplied args)
				}

				numProperty->m_min = min(numProperty->m_min, numProperty->GetFloatValues()[0]);
				numProperty->m_max = max(numProperty->m_max, numProperty->GetFloatValues()[0]);


				if(m_notchBlock.m_demolitionGetPropertyDMXOffset)
				{
					numProperty->m_dmxOffset = m_notchBlock.m_demolitionGetPropertyDMXOffset(propertyId);
				}
				if(m_notchBlock.m_demolitionGetPropertyDMX16Bit)
				{
					numProperty->m_dmxIs16Bit = m_notchBlock.m_demolitionGetPropertyDMX16Bit(propertyId);
				}
				
				m_properties.push_back(numProperty);
			}
			else
			{
				// error
			}	
		}
	}

	return true;

}
bool NotchInstance::InitLayers()
{
	int documentHandle = m_instanceId;
	if(m_notchBlock.m_demolitionSetCurrentDocumentHandle)
	{
		m_notchBlock.m_demolitionSetCurrentDocumentHandle(documentHandle);
	}

	
	for(size_t i = 0; i < m_layers.size(); ++i)
	{
		Layer* layer = m_layers[i];
		if(layer)
		{
			delete layer;
		}
	}
	m_layers.clear();

	
	// iterate layers if available
	if(m_layerSelectionProperty)
	{
		for(size_t i = 0; i < m_layerSelectionProperty->m_enums.size(); ++i)
		{
			char layerGuid[64];
			if(m_notchBlock.m_demolitionGetLayerGUID(layerGuid, (int)i) != 0)
			{
				Layer* layer = new Layer;
				layer->m_guid = layerGuid;
				layer->m_name = m_layerSelectionProperty->m_enums[i];
				layer->m_index = (int)i;


				if (m_notchBlock.m_demolitionIsPropertyActiveInLayer)
				{
					for (size_t j = 0; j < m_properties.size(); ++j)
					{
						if (m_notchBlock.m_demolitionIsPropertyActiveInLayer(m_properties[j]->m_propertyId.c_str(), layerGuid))
						{
							layer->m_properties.push_back(m_properties[j]);
						}
					}
				}
				else
				{
					// no filter function exists - push all the properties on the list for every layer
					for (size_t j = 0; j < m_properties.size(); ++j)
					{
						layer->m_properties.push_back(m_properties[j]);
					}
				}
			

				m_layers.push_back(layer);
			}
		}
		if(m_layers.size())
		{
			SetCurrentLayer(m_layers[0]->m_guid.c_str());
		}
	}

	return true;
}
