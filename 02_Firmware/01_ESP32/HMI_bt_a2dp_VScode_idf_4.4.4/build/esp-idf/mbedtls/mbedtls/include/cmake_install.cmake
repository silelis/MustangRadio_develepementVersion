# Install script for directory: C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/HMI_bt_a2dp_VScode_idf_4.4.4")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/Users/dbank/.espressif/tools/xtensa-esp32-elf/esp-2021r2-patch5-8.4.0/xtensa-esp32-elf/bin/xtensa-esp32-elf-objdump.exe")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/aes.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/aesni.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/arc4.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/aria.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/asn1.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/asn1write.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/base64.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/bignum.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/blowfish.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/bn_mul.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/camellia.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ccm.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/certs.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/chacha20.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/chachapoly.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/check_config.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/cipher.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/cipher_internal.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/cmac.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/compat-1.3.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/config.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/config_psa.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/constant_time.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ctr_drbg.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/debug.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/des.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/dhm.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ecdh.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ecdsa.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ecjpake.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ecp.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ecp_internal.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/entropy.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/entropy_poll.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/error.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/gcm.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/havege.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/hkdf.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/hmac_drbg.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/md.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/md2.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/md4.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/md5.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/md_internal.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/memory_buffer_alloc.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/net.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/net_sockets.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/nist_kw.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/oid.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/padlock.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/pem.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/pk.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/pk_internal.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/pkcs11.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/pkcs12.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/pkcs5.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/platform.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/platform_time.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/platform_util.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/poly1305.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/psa_util.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ripemd160.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/rsa.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/rsa_internal.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/sha1.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/sha256.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/sha512.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ssl.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ssl_cache.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ssl_ciphersuites.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ssl_cookie.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ssl_internal.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/ssl_ticket.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/threading.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/timing.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/version.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/x509.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/x509_crl.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/x509_crt.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/x509_csr.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/mbedtls/xtea.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/psa" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_builtin_composites.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_builtin_primitives.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_compat.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_config.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_driver_common.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_driver_contexts_composites.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_driver_contexts_primitives.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_extra.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_platform.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_se_driver.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_sizes.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_struct.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_types.h"
    "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/mbedtls/mbedtls/include/psa/crypto_values.h"
    )
endif()

