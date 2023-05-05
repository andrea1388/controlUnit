idf_component_register(SRCS "Proto485.cpp" "Switch.cpp" "BinarySensor.cpp" "millis.cpp" "TempSens.cpp" "ControlUnit.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES Esp32Ds18b20Component Esp32NvsComponent
)
