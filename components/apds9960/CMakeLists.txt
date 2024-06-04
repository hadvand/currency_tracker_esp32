set(component_srcs "apds9960.c")

idf_component_register(SRCS "${component_srcs}"
                        PRIV_REQUIRES driver bus
                        INCLUDE_DIRS ".")