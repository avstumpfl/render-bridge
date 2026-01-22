
#include "Input.h"
#include "Instance.h"
#include "glm/gtx/quaternion.hpp"
#include "Extension.h"

namespace rxext::notch {

namespace {
  // source: https://github.com/recp/cglm/blob/master/include/cglm/cam.h
  void glm_persp_decomp(const glm::mat4& proj,
      float* nearVal, float* farVal,
      float* top,     float* bottom,
      float* left,    float* right) {
    float m00, m11, m20, m21, m22, m32, n, f;
    float n_m11, n_m00;

    m00 = proj[0][0];
    m11 = proj[1][1];
    m20 = proj[2][0];
    m21 = proj[2][1];
    m22 = proj[2][2];
    m32 = proj[3][2];

    n = m32 / (m22 - 1.0f);
    f = m32 / (m22 + 1.0f);

    n_m11 = n / m11;
    n_m00 = n / m00;

    *nearVal = n;
    *farVal  = f;
    *bottom  = n_m11 * (m21 - 1.0f);
    *top     = n_m11 * (m21 + 1.0f);
    *left    = n_m00 * (m20 - 1.0f);
    *right   = n_m00 * (m20 + 1.0f);
  }

  void set_property_Int(NotchInstance* instance, 
      NotchExposedPropertyInt* property,
      ParameterInt* parameter) {
    instance->SetIntProperty(property, parameter->value());
  }

  void set_property_Float(NotchInstance* instance, 
      NotchExposedPropertyFloat* property,
      ParameterValue* parameter) {
    instance->SetFloatProperty(property, 
      static_cast<float>(parameter->value()));
  }

  void set_property_Float2(NotchInstance* instance, 
      NotchExposedPropertyFloat* property,
      ParameterVector2* parameter) {
    auto values = property->GetFloatValues();
    values[0] = static_cast<float>(parameter->value()[0]);
    values[1] = static_cast<float>(parameter->value()[1]);
    instance->SetFloatProperty(property, values, property->m_numChannels);
  }

  void set_property_Float3(NotchInstance* instance, 
      NotchExposedPropertyFloat* property,
      ParameterVector3* parameter) {
    auto values = property->GetFloatValues();
    values[0] = static_cast<float>(parameter->value()[0]);
    values[1] = static_cast<float>(parameter->value()[1]);
    values[2] = static_cast<float>(parameter->value()[2]);
    instance->SetFloatProperty(property, values, property->m_numChannels);
  }

  void set_property_Float4(NotchInstance* instance, 
      NotchExposedPropertyFloat* property,
      ParameterVector4* parameter) {
    auto values = property->GetFloatValues();
    values[0] = static_cast<float>(parameter->value()[0]);
    values[1] = static_cast<float>(parameter->value()[1]);
    values[2] = static_cast<float>(parameter->value()[2]);
    values[3] = static_cast<float>(parameter->value()[3]);
    instance->SetFloatProperty(property, values, property->m_numChannels);
  }

  void set_property_String(NotchInstance* instance, 
      NotchExposedPropertyString* property,
      ParameterString* parameter) {
    instance->SetStringProperty(property, parameter->value().c_str());
  }

  void set_property_Texture(rxext::HostContext host, NotchInstance* instance, 
      d3d11::Device* device, NotchExposedPropertyImage* property,
      ParameterTexture* parameter) {

    update_input_texture(*parameter, [&](rxext::TextureDesc desc) {
      return parameter->create(host, *device, desc);
    });

    if (parameter->d3d11_texture())
      instance->SetTextureProperty(property, parameter->d3d11_texture(), 
        parameter->d3d11_shader_resource_view());
  }

  void set_property_ExposedNull(NotchInstance* instance, 
      NotchExposedPropertyFloat* property,
      ParameterVector3* position, ParameterVector3* rotation) {
    auto values = property->GetFloatValues();
    auto quat = glm::dquat(rotation->value());
    values[0] = static_cast<float>(position->value().x);
    values[1] = static_cast<float>(position->value().y);
    values[2] = static_cast<float>(position->value().z);
    values[3] = static_cast<float>(quat.x);
    values[4] = static_cast<float>(quat.y);
    values[5] = static_cast<float>(quat.z);
    values[6] = static_cast<float>(quat.w);
    instance->SetFloatProperty(property, values, property->m_numChannels);
  }

  void set_property_PosRotScale(NotchInstance* instance, 
      NotchExposedPropertyFloat* property,
      ParameterVector3* position, ParameterVector3* rotation, ParameterVector3* scale) {
    auto values = property->GetFloatValues();
    auto i = 0;
    values[i++] = static_cast<float>(position->value().x);
    values[i++] = static_cast<float>(position->value().y);
    values[i++] = static_cast<float>(position->value().z);
    if (property->GetPropertyType() == NotchExposedProperty::PropertyType::PropertyType_PosRotQuatScale) {
      auto quat = glm::dquat(rotation->value());
      values[i++] = static_cast<float>(quat.x);
      values[i++] = static_cast<float>(quat.y);
      values[i++] = static_cast<float>(quat.z);
      values[i++] = static_cast<float>(quat.w);
    }
    else {
      values[i++] = static_cast<float>(rotation->value().x);
      values[i++] = static_cast<float>(rotation->value().y);
      values[i++] = static_cast<float>(rotation->value().z);
    }
    values[i++] = static_cast<float>(scale->value().x);
    values[i++] = static_cast<float>(scale->value().y);
    values[i++] = static_cast<float>(scale->value().z);
    instance->SetFloatProperty(property, values, property->m_numChannels);
  }

  void set_property_ExposedCamera(NotchInstance* instance, 
      NotchExposedPropertyFloat* property, 
      ParameterMatrix4* view_matrix, 
      ParameterMatrix4* world_matrix, 
      ParameterMatrix4* projection_matrix) {
    
    const auto model_matrix = view_matrix->value() * world_matrix->value();
    const auto rotation = glm::inverse(glm::quat{ model_matrix });
    const auto translation = -(rotation * glm::vec4(model_matrix[3]));
    float near, far, top, bottom, left, right;
    glm_persp_decomp(projection_matrix->value(), &near, &far, &top, &bottom, &left, &right);
		const auto w = (right - left);
		const auto h = (top - bottom);
		const auto scale_x = 2.0f * near / w;
		const auto scale_y = 2.0f * near / h;
		const auto offset_x = -1.0f * (right + left) / w;
		const auto offset_y = -1.0f * (top + bottom) / h;

    auto values = property->GetFloatValues();
    values[0] = translation.x;
    values[1] = translation.y;
    values[2] = translation.z;
    values[3] = rotation.x;
    values[4] = rotation.y;
    values[5] = rotation.z;
    values[6] = rotation.w;
    values[7] = scale_x;
    values[8] = scale_y;
    values[9] = near;
    values[10] = far;
    values[11] = offset_x;
    values[12] = offset_y;
    instance->SetFloatProperty(property, values, property->m_numChannels);
  }

  void set_property_Matrix4(NotchInstance* instance, 
      NotchExposedPropertyFloat* property,
      ParameterMatrix4* parameter) {
    const auto& matrix = parameter->value();
    auto values = property->GetFloatValues();
    for (auto i = 0, d = 0; i < 4; ++i)
      for (auto j = 0; j < 4; ++j)
        values[d++] = static_cast<float>(matrix[i][j]);
    instance->SetFloatProperty(property, values, property->m_numChannels);
  }

  template<typename T>
  const T& get_value(const NotchExposedPropertyFloat* notch_property) {
    assert(notch_property->m_numChannels * sizeof(float) == sizeof(T));
    return *reinterpret_cast<const T*>(notch_property->GetFloatValues());
  }
} // namespace

Input::Input(std::shared_ptr<Instance> instance, ValueSet settings) 
  : m_instance(std::move(instance)),
    m_settings(std::move(settings)),
    m_use_property_ids(m_settings.get<bool>("use_property_ids")) {
  m_instance->register_input(this);
}

Input::~Input() {
  m_instance->deregister_input(this);
}

bool Input::initialize() noexcept try {
  const auto layer_id = m_settings.get(SettingNames::layer_id);
  if (!layer_id.empty())
    for (auto layer : m_instance->notch_instance().GetLayers())
      if (layer->m_guid == layer_id)
        m_instance->notch_instance().SetLayer(layer);

  const auto layer_index = add_parameter<ParameterInt>("layer_index", 0);
  layer_index->set_property(PropertyNames::purpose, PurposeNames::LayerIndex);
  if (layer_id.empty())
    m_layer_index = layer_index;

  m_time = add_parameter<ParameterValue>("time");
  m_time->set_property(PropertyNames::purpose, PurposeNames::TimelineTime);

  m_visible = add_internal_parameter<ParameterBool>("visible", true);
  m_visible->set_property(PropertyNames::purpose, PurposeNames::Visible);

  add_internal_parameter<ParameterValue>("alpha", 1.0);

  m_sampler = add_output_parameter<ParameterTexture>("sampler");

  const auto& properties = (layer_id.empty() ? 
    m_instance->notch_instance().GetAllProperties() :
    m_instance->notch_instance().GetActiveProperties());

  for (auto property : properties) {
    m_added_parameters.clear();

    switch (property->GetPropertyDataType()) {
      case NotchExposedProperty::PropertyDataType_Float:
        add_property_parameters(static_cast<NotchExposedPropertyFloat*>(property));
        break;
      case NotchExposedProperty::PropertyDataType_Int:
        add_property_parameters(static_cast<NotchExposedPropertyInt*>(property));
        break;
      case NotchExposedProperty::PropertyDataType_String:
        add_property_parameters(static_cast<NotchExposedPropertyString*>(property));
        break;
      case NotchExposedProperty::PropertyDataType_Texture:
        add_property_parameters(static_cast<NotchExposedPropertyImage*>(property));
        break;
    }

    // set properties of currently added parameters
    auto active_in_layers = std::vector<int>();
    const auto layers = m_instance->notch_instance().GetLayers();
    for (auto index = 0; index < static_cast<int>(layers.size()); ++index) {
      const auto& layer_properties = layers[index]->m_properties;
      if (std::count(begin(layer_properties), end(layer_properties), property))
        active_in_layers.push_back(index);
    }
    for (auto parameter : m_added_parameters) {
      parameter->set_property(PropertyNames::group_name, property->m_groupName);
      parameter->set_property(PropertyNames::active_in_layers, active_in_layers);

      if (property->GetPropertyDataType() == NotchExposedProperty::PropertyDataType_Float) {
        auto float_property = static_cast<const NotchExposedPropertyFloat*>(property);
        parameter->set_property(PropertyNames::min_value, float_property->m_min);
        parameter->set_property(PropertyNames::max_value, float_property->m_max);
        if (float_property->GetPropertyType() == NotchExposedProperty::PropertyType_Colour)
          parameter->set_property(PropertyNames::purpose, PurposeNames::Color);
      }
      else if (property->GetPropertyDataType() == NotchExposedProperty::PropertyDataType_Int) {
        auto int_property = static_cast<const NotchExposedPropertyInt*>(property);
        parameter->set_property(PropertyNames::enum_names, int_property->m_enums);
      }
    }
  }

  // TODO: remove when Pixera does not set these unconditionally
  for (const auto name : { "camera_index" })
    if (!find_parameter(name))
        add_parameter<ParameterInt>(name);
  for (const auto name : { "view_matrix", "world_matrix", "projection_matrix" })
    if (!find_parameter(name))
      add_parameter<ParameterMatrix4>(name, glm::mat4x4(1));

  return update_settings(std::move(m_settings));
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return false;
}

bool Input::update_settings(ValueSet settings) noexcept try {
  m_settings = std::move(settings);
  m_render_interval_manager.set_target_frame_rate(
    m_settings.get<double>(SettingNames::frame_rate));
  m_max_time_elapsed = m_settings.get<double>("max_time_elapsed", 0.050);

  auto desc = TextureDesc{ };
  desc.width = m_settings.get<int>(SettingNames::resolution_x, 256);
  desc.height = m_settings.get<int>(SettingNames::resolution_y, 256);
  desc.format = get_format_by_name(m_settings.get(SettingNames::format));
  desc.is_target = true;
  if (m_settings.get<bool>(SettingNames::preview)) {
    desc.width /= 4;
    desc.height /= 4;
  }
  m_sampler->create(host(), m_instance->d3d11_device(), desc);
  return true;
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return false;
}

string Input::get_property(string_view name) noexcept {
  if (name == PropertyNames::shader_export)
    return string("NotchSampler");
  if (name == PropertyNames::shader_file)
    return string("data/shaders/NotchSampler.glsl");
  if (name == PropertyNames::layer_names)
    return value_to_string(m_instance->get_layer_names());
  if (name == PropertyNames::layer_ids)
    return value_to_string(m_instance->get_layer_ids());

  return { };
}

std::string Input::get_new_parameter_name(
      const std::string& base_name, const std::string& attribute) {
  const auto get_name = [&](int index) {
    auto name = base_name;
    if (index > 1)
      name += " [" + std::to_string(index) + "]";
    if (!attribute.empty())
      name += " (" + attribute + ")";
    return name;
  };
  for (auto i = 1; ; ++i) {
    auto name = get_name(i);
    if (!find_parameter(name))
      return name;
  }
}

auto Input::get_new_parameter_name(const NotchExposedProperty* property, 
    const std::string& attribute) -> Ident {
  if (m_use_property_ids) {
    auto name = property->m_name;
    if (!attribute.empty())
      name += " (" + attribute + ")";
    return { get_new_parameter_name(property->m_propertyId, attribute), name, attribute };
  }
  return { get_new_parameter_name(property->m_name, attribute), "", attribute };
}

void Input::add_property_parameters(NotchExposedPropertyFloat* notch_property) {
  const auto get_name = [&](const std::string& attribute = "") { 
    return get_new_parameter_name(notch_property, attribute); 
  };
  switch (notch_property->GetPropertyType()) {
    case NotchExposedProperty::PropertyType_Float:
    case NotchExposedProperty::PropertyType_Colour:
      switch (notch_property->m_numChannels) {
        case 1:
          m_property_updaters.push_back(
            std::bind(&set_property_Float, 
              &m_instance->notch_instance(), notch_property,
              add_parameter<ParameterValue>(get_name(), 
                get_value<float>(notch_property))));
          break;
         
        case 2:
          m_property_updaters.push_back(
            std::bind(&set_property_Float2, 
              &m_instance->notch_instance(), notch_property,
              add_parameter<ParameterVector2>(get_name(),
                get_value<glm::vec2>(notch_property))));
          break;

        case 3:
          m_property_updaters.push_back(
            std::bind(&set_property_Float3, 
              &m_instance->notch_instance(), notch_property,
              add_parameter<ParameterVector3>(get_name(),
                get_value<glm::vec3>(notch_property))));
          break;

        case 4:
          m_property_updaters.push_back(
            std::bind(&set_property_Float4, 
              &m_instance->notch_instance(), notch_property,
              add_parameter<ParameterVector4>(get_name(),
                get_value<glm::vec4>(notch_property))));
          break;
      }
      break;

    case NotchExposedProperty::PropertyType_ExposedNull:
      m_property_updaters.push_back(
        std::bind(&set_property_ExposedNull, 
          &m_instance->notch_instance(), notch_property,
          add_parameter<ParameterVector3>(get_name(PurposeNames::Position)),
          add_parameter<ParameterVector3>(get_name(PurposeNames::Rotation))));
      break;

		case NotchExposedProperty::PropertyType_ExposedCamera:
      m_property_updaters.push_back(
        std::bind(&set_property_ExposedCamera, 
          &m_instance->notch_instance(), notch_property,
          add_parameter<ParameterMatrix4>(get_name(PurposeNames::ViewMatrix), glm::mat4x4(1)),
          add_parameter<ParameterMatrix4>(get_name(PurposeNames::WorldMatrix), glm::mat4x4(1)),
          add_parameter<ParameterMatrix4>(get_name(PurposeNames::ProjectionMatrix), glm::mat4x4(1))));
      break;
 
		case NotchExposedProperty::PropertyType_PosRotScale:
    case NotchExposedProperty::PropertyType_PosRotQuatScale:
      m_property_updaters.push_back(
        std::bind(&set_property_PosRotScale, 
          &m_instance->notch_instance(), notch_property,
          add_parameter<ParameterVector3>(get_name(PurposeNames::Position)),
          add_parameter<ParameterVector3>(get_name(PurposeNames::Rotation)),
          add_parameter<ParameterVector3>(get_name(PurposeNames::Scale), glm::dvec3(1))));
      break;

    case NotchExposedProperty::PropertyType_Matrix4x4:
      m_property_updaters.push_back(
        std::bind(&set_property_Matrix4, 
          &m_instance->notch_instance(), notch_property,
          add_parameter<ParameterMatrix4>(get_name(),
            get_value<glm::mat4x4>(notch_property))));
      break;

    default:
      assert(!"unsupported property type");
  }
}

void Input::add_property_parameters(NotchExposedPropertyInt* notch_property) {
  m_property_updaters.push_back(
    std::bind(&set_property_Int, 
      &m_instance->notch_instance(), notch_property,
      add_parameter<ParameterInt>(
        get_new_parameter_name(notch_property), 
          notch_property->m_intValue)));
}

void Input::add_property_parameters(NotchExposedPropertyString* notch_property) {
  m_property_updaters.push_back(
    std::bind(&set_property_String, 
      &m_instance->notch_instance(), notch_property,
      add_parameter<ParameterString>(
        get_new_parameter_name(notch_property), 
          notch_property->m_value)));
}

void Input::add_property_parameters(NotchExposedPropertyImage* notch_property) {
  m_property_updaters.push_back(
    std::bind(&set_property_Texture, 
      host(),
      &m_instance->notch_instance(), 
      &m_instance->d3d11_device(),
      notch_property,
      add_parameter<ParameterTexture>(
        get_new_parameter_name(notch_property))));
}

bool Input::update() noexcept try { 
  // skip update/rendering when a target frame rate is set
  if (!m_render_interval_manager.update())
    return false;

  // simulation is reset, when a negative time is set (documentation is wrong!)
  // do this once when jumping to zero
  const auto time = (!m_time->value() && m_prev_time ? -1.0 : m_time->value());
  const auto time_elapsed = std::clamp(time - m_prev_time, 0.0, m_max_time_elapsed);
  m_prev_time = time;

  // update while visible or when time changed
  if (!m_visible->value() && time_elapsed == 0)
    return false;

  // only update time and other properties once per instance
  if (m_instance->is_first_input(this)) {
    // set current layer
    const auto& layers = m_instance->notch_instance().GetLayers();
    if (m_layer_index && !layers.empty())
      m_instance->notch_instance().SetLayer(
        layers[std::clamp(m_layer_index->value(), 0, static_cast<int>(layers.size()) - 1)]);

    for (const auto& update : m_property_updaters)
      update();

    m_instance->notch_instance().SetTime(time, time_elapsed);
  }
  return true;
}
catch (const std::exception& ex) {
  host().log_error(ex.what());
  return false;
}

SyncDesc Input::before_render() noexcept {
  return m_instance->get_sync_desc();
}

RenderResult Input::render() noexcept {
  return m_instance->render(*m_sampler);
}

SyncDesc Input::after_render() noexcept {
  return m_instance->get_sync_desc();
}

} // namespace
