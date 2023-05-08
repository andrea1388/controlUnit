idf_component_register(SRCS "Switch.cpp" "BinarySensor.cpp" "millis.cpp" "TempSens.cpp" "ControlUnit.cpp" "Proto485.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES Esp32Ds18b20Component Esp32NvsComponent
)
