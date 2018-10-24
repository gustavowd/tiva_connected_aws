/*
 * PEM-encoded client certificate.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----"
 * "...base64 data..."
 * "-----END CERTIFICATE-----";
 */
static const char clientcredentialCLIENT_CERTIFICATE_PEM[] = 
"-----BEGIN CERTIFICATE-----\n"
"MIIDWTCCAkGgAwIBAgIUKq72F0jVbMLciQGq+qmnpo0W9XgwDQYJKoZIhvcNAQEL\n"
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE4MTAyMzE2NDEz\n"
"NVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOtFT023k/kP4WVYC9BX\n"
"2c8G0G/V6udg4FhsZxQ7Fh0R76JGQJrqXFTQ2UWgWCtFyK8C9TWS4vVD5ZHoFrNn\n"
"jCYrdTO9gZalQqey6ogIdHH5ojQNt+izZbp1y+hGO2JSOZ+D+LBdVrFZyHC5k9D7\n"
"JkO+dVDYP3F9kCJimXm1nfeW0UYR/AtJ2pmCtjbOm6ct6R3aoeY5FfWy5AkQyYmh\n"
"dfVwIzZRip/RIwh5vdrzV6iqMiF0m9820Q3nAgB6V5veWL2kQZTgiLmY7yY0V7hG\n"
"zZdch3YmwsX45lv1aizmbBSyiWUdZBO9vVwewOW2mPK9yYP6kDfehUw81sy3yzbu\n"
"XQcCAwEAAaNgMF4wHwYDVR0jBBgwFoAUwq2MkwqDO4o/d/sRjZQVIyiF5FowHQYD\n"
"VR0OBBYEFC4K/7KqoZcUemr4+et7knpkp/XiMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQA+GfnW4jaxpuze4HSXGgNd/ijg\n"
"wsQtjtboGaAEWqHiEbyYfyPusSBHcPouuwRavCjEcdh6ncMInEoxIsjBzZ2pyqXR\n"
"MqyCUU5sdobqOFeRE5tb/pRjm0G0J8zps/nJvpFUNw+0wxdGsrIKNRO3Ki9w0sht\n"
"/sYxcZp8e0MTsCMraB1VeE/+zMhRemGglsqGO5lOmH71XP8oJBuR49/wAtilhW8F\n"
"azQg02cfiXYjVNTlpsc17u9j3/nMeIwdlQ7wKABv+EczYeqr6Tg4/G/Da0ObDL6W\n"
"gdDcsR6A1qXFdrvbSGgZENRD16Gc9hf0aDQk+emwO2GjDKpnIsDG5ASXTTXZ\n"
"-----END CERTIFICATE-----";

/*
 * PEM-encoded client private key.
 *
 * Must include the PEM header and footer:
 * "-----BEGIN RSA PRIVATE KEY-----"
 * "...base64 data..."
 * "-----END RSA PRIVATE KEY-----";
 */
static const char clientcredentialCLIENT_PRIVATE_KEY_PEM[] =
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEA60VPTbeT+Q/hZVgL0FfZzwbQb9Xq52DgWGxnFDsWHRHvokZA\n"
"mupcVNDZRaBYK0XIrwL1NZLi9UPlkegWs2eMJit1M72BlqVCp7LqiAh0cfmiNA23\n"
"6LNlunXL6EY7YlI5n4P4sF1WsVnIcLmT0PsmQ751UNg/cX2QImKZebWd95bRRhH8\n"
"C0namYK2Ns6bpy3pHdqh5jkV9bLkCRDJiaF19XAjNlGKn9EjCHm92vNXqKoyIXSb\n"
"3zbRDecCAHpXm95YvaRBlOCIuZjvJjRXuEbNl1yHdibCxfjmW/VqLOZsFLKJZR1k\n"
"E729XB7A5baY8r3Jg/qQN96FTDzWzLfLNu5dBwIDAQABAoIBAHFKMMPVsHVwRjMM\n"
"0LOEeieMunX/5luAdMdfe8tsIFvjNSdI+Pcf/1IAN3+llI9RzS8pF3aywoMkLLmz\n"
"rqq5I+2FY6yEhpBKB+zE5NaYa6oucQLr0DFHoabF7mhs2U+hFIpRfEFA0OwZtcFC\n"
"O4GYTna0kb1aREKcGfnUzN6kq+p6UtME84IDNNpFPO35SU6MvcFlhXPkPmrgRfuA\n"
"f7QF8RlXF1V1XCfjbo0yHxdn+PKX6m1uY0kyCTiVK5QyZ5/IBXQ/5cqYOnr4MN1i\n"
"ta2swz3iddiLWoTjF9ink5aJV4EFfY0colV35/B42gaEXQWs+iRZmrEKcGR395A2\n"
"+ILhouECgYEA/cOg9ZbrvumAxTJRcs5Q0q3EWi0CxuZe7iMI3Q+CPUmqIynF39Wd\n"
"XOiEWhePpqZoOXVCmIzak+tQVqQDdT3iJuKblCkN8XSmax6e0lJhvjEGhp4cKUPj\n"
"2aKDHG9E8FIbY/ybq7NrvMcXJN0mxvvQ8Gwv2vmh8YZGWROHRv1VoZ8CgYEA7Vf3\n"
"+RP9nc8bwrAbCsTb4nRok/9qkGrscaAVwMwriuaJGw7DPOCxMub9hAEdDIIUkW2D\n"
"25N5HUQWkAdaWPa4vbzQTGGBAeaM0w+FZuTDfC1Tr3jP/VVCGRr9jZkN3C1N8DiD\n"
"q+55UWx6sv7jmup3EARXWKF4YkQmpNq8kr8rG5kCgYEAuGdYqB6r4We9c49FMLBM\n"
"e0xay0y6H994TISZBEso0XCeej7409dfKwysYOuOMAPs6RQU+NYiYCs3jj5RZPk7\n"
"RhaWB+xtfXDPHjh6IUxMSvFFVL3AqRv0j0pRDHCvIBQKY3ZFiVgwilYCs38hlqeX\n"
"BZ8Yh3+v9tskc8aFH0ko2PECgYB0Fn3kZ2G6KU3juO1RJB6EsflgwiImRxYlMW7S\n"
"YUq+4qmXAW5/PUuOoSrIwR6rOojC5PUElio8JUx/4AjR0MNlko+zfFF0U4b3yPSv\n"
"8h6ngygWgJoCyAvPacMw2fbMhaBEafAFTVwrdkrBeChtYlcgD2+Fm1zde4YHRjHL\n"
"+RMlqQKBgQDylHzV0weTxw2Q+wu0Gx2muNa/gJXkVVfkDEuMEq767Zoz9t1xpDM7\n"
"uQidivsLy3MUYA6qerAXlD5xNiMJrI5rht2kmhOZNs/dGLIL2l0elPy66wYHQgzw\n"
"y1ngd3MQKAOdwrz8585vMThA7vGUjc+P2pebDG6ZtzIR5PCU+jYoVw==\n"
"-----END RSA PRIVATE KEY-----";

/*
 * PEM-encoded Just-in-Time Registration (JITR) certificate (optional).
 *
 * If used, must include the PEM header and footer:
 * "-----BEGIN CERTIFICATE-----"
 * "...base64 data..."
 * "-----END CERTIFICATE-----";
 */
static const char * clientcredentialJITR_DEVICE_CERTIFICATE_AUTHORITY_PEM = NULL;