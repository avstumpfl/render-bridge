#pragma once

#include "rxext_client.h"
#include "Parameter.h"
#include "notch/NotchBlock.h"
#include "util/RenderIntervalManager.h"
#include <functional>
#include <vector>

namespace rxext::notch {

class Instance;

class Input : public rxext::InputStream {
public:
  Input(std::shared_ptr<Instance> instance, ValueSet settings);
  ~Input();

  bool initialize() noexcept override;
  bool update_settings(ValueSet settings) noexcept override;
  string get_property(string_view name) noexcept override;
  bool update() noexcept override;
  SyncDesc before_render() noexcept override;
  RenderResult render() noexcept override;
  SyncDesc after_render() noexcept override;

private:
  struct Ident {
    std::string name;
    std::string friendly;
    std::string purpose;

    Ident(const char* name) : name(name) { }
    Ident(std::string name) : name(std::move(name)) { }
    Ident(std::string name, std::string friendly, std::string purpose) 
      : name(std::move(name)), friendly(std::move(friendly)), purpose(std::move(purpose)) { }
  };

  std::string get_new_parameter_name(const std::string& base_name,
    const std::string& attribute = {});
  Ident get_new_parameter_name(const NotchExposedProperty* property,
    const std::string& attribute = {});
  void add_property_parameters(NotchExposedPropertyFloat* notch_property);
  void add_property_parameters(NotchExposedPropertyInt* notch_property);
  void add_property_parameters(NotchExposedPropertyString* notch_property);
  void add_property_parameters(NotchExposedPropertyImage* notch_property);

  template<typename T, typename... Args>
  T* add_parameter(const Ident& ident, Args&&... args) {
    auto p = rxext::InputStream::add_parameter<T>(ident.name, std::forward<Args>(args)...);
    m_added_parameters.push_back(p);
    if (!ident.friendly.empty())
      p->set_property(PropertyNames::name, ident.friendly);
    if (!ident.purpose.empty())
      p->set_property(PropertyNames::purpose, ident.purpose);
    return p;
  }

  template<typename T, typename... Args>
  T* add_internal_parameter(const Ident& ident, Args&&... args) {
    auto p = add_parameter<T>(ident, std::forward<Args>(args)...);
    p->set_property(PropertyNames::internal, true);
    return p;
  }

  std::shared_ptr<Instance> m_instance;
  ValueSet m_settings;
  const bool m_use_property_ids{ };
  std::vector<Parameter*> m_added_parameters;
  std::vector<std::function<void()>> m_property_updaters;
  ParameterValue* m_time{ };
  ParameterBool* m_visible{ };
  ParameterInt* m_layer_index{ };
  ParameterTexture* m_sampler{ };
  util::RenderIntervalManager m_render_interval_manager;
  double m_prev_time{ };
  double m_max_time_elapsed{ 0.050 };
};

} // namespace
