#pragma once

#include "common/Duration.h"
#include "common/statistics.h"
#include <cmath>

namespace util {
  
class RenderIntervalManager {
private:
  double m_target_frame_rate{ };
  common::Duration m_last_present_time{ };
  common::ExponentialStatistic<common::Duration> m_frame_duration{ };
  common::Duration m_last_frame_time{ };
  int m_render_interval_counter{ };
  
  void update_frame_duration() {
    const auto now = static_cast<common::Duration>(
      std::chrono::high_resolution_clock::now().time_since_epoch());
    if (m_last_present_time.count()) {
      const auto frame_duration = now - m_last_present_time;
      if (!m_frame_duration.mean().count())
        m_frame_duration.reset(frame_duration);
      else
        m_frame_duration.push(frame_duration, 0.01);
    }
    m_last_present_time = now;
  }

  bool update_render_interval() {
    if (!m_target_frame_rate)
      return true;

    const auto actual_frame_rate = std::round(1.0 / m_frame_duration.mean().count());
    if (!actual_frame_rate || !std::isfinite(actual_frame_rate))
      return false;
    if (m_target_frame_rate > actual_frame_rate)
      return true;

    // skip exactly every nth frame when there is an almost integral fraction
    const auto ratio = (actual_frame_rate / m_target_frame_rate);
    const auto fraction = std::fmod(ratio, 1.0);
    const auto swap_interval = static_cast<int>(std::round(ratio));
    if ((swap_interval > 1 && swap_interval < 10) && 
        (fraction > 0.9 || fraction + 1 < 1.1)) {
      if (m_render_interval_counter++ % swap_interval != 0)
        return false;
    }
    else {
      const auto now = static_cast<common::Duration>(
        std::chrono::high_resolution_clock::now().time_since_epoch());
      const auto frame_duration = common::Duration(1.0 / m_target_frame_rate);
      if (now < m_last_frame_time + frame_duration)
        return false;
      m_last_frame_time += frame_duration;
      if (m_last_frame_time + frame_duration <= now)
        m_last_frame_time = now;
    }
    return true;
  }

public:
  void set_target_frame_rate(double frame_rate) {
    m_target_frame_rate = frame_rate;
  }

  bool update() {
    update_frame_duration();
    return update_render_interval();
  }
};

} // namespace
