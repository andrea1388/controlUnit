idf_component_register(SRCS "millis.cpp" "TempSens.cpp" "ControlUnit.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES Esp32Ds18b20Component
)
