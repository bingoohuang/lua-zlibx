package.path = "./?.lua;" .. package.path
package.cpath = "./?.so;" .. package.cpath

local lz = require("zlib")

local sm3 = lz.sm3_base64("data_lqlq666lqlq946")
print(sm3)
-- H9Yuyh2qqGFlYC/HEKl0Ir809GOYuECfLoiRO1sUZDA=

local sm3hmac = lz.sm3hmac_base64("data_lqlq666lqlq946", "key_123")
print(sm3hmac)
-- 8IPopN0cpqHinIkPb0bXTqnq+suVlVYEeCjGbHGlDTk=
