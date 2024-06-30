#include "homeassistant_switch.h"
#include "esphome/core/log.h"
#include "esphome/components/api/api_server.h"

namespace esphome {
namespace homeassistant {

static const char *const TAG = "homeassistant.switch";

using namespace esphome::switch_;

void HomeassistantSwitch::setup() {
  api::global_api_server->subscribe_home_assistant_state(
      this->entity_id_, optional<std::string>(""), [this](const std::string &state) {
        auto val = parse_on_off(state.c_str());
        switch (val) {
          case PARSE_NONE:
          case PARSE_TOGGLE:
            ESP_LOGW(TAG, "Can't convert '%s' to binary state!", state.c_str());
            break;
          case PARSE_ON:
          case PARSE_OFF:
            bool new_state = val == PARSE_ON;
            ESP_LOGD(TAG, "'%s': Got state %s", this->entity_id_.c_str(), ONOFF(new_state));
            this->publish_state(new_state);
            break;
        }
        return;
      });
}

void HomeassistantSwitch::dump_config() {
  LOG_SWITCH("", "Homeassistant Switch", this);
  ESP_LOGCONFIG(TAG, "  Entity ID: '%s'", this->entity_id_.c_str());
}

float HomeassistantSwitch::get_setup_priority() const { return setup_priority::AFTER_CONNECTION; }

void HomeassistantSwitch::write_state(bool state) {
  if (!api::global_api_server) {
    ESP_LOGE(TAG, "Missing API Server");
    return;
  }
  if (!api::global_api_server->is_connected()) {
    ESP_LOGE(TAG, "API Server not connected");
    return;
  }

  api::HomeassistantServiceResponse resp;
  api::HomeassistantServiceMap entity_id_kv;
  entity_id_kv.key = "entity_id";
  entity_id_kv.value = this->entity_id_;
  resp.data.push_back(entity_id_kv);

  if (state) {
    resp.service = "switch.turn_on";
  } else {
    resp.service = "switch.turn_off";
  }

  api::global_api_server->send_homeassistant_service_call(resp);
}

}  // namespace homeassistant
}  // namespace esphome
