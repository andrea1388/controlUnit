idf_component_register(SRCS "ota.cpp" "Switch.cpp" "BinarySensor.cpp" "millis.cpp" "TempSens.cpp" "ControlUnit.cpp" "Proto485.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES esp_https_ota Esp32Ds18b20Component Esp32NvsComponent Esp32WiFiComponent
)
