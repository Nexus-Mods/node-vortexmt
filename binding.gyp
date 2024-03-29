{
    "targets": [
        {
            "target_name": "vortexmt",
            "includes": [
                "auto.gypi"
            ],
            "sources": [
                "src/common.h",
                "src/md5.h",
                "src/md5.cpp",
                "src/main.cpp"
            ],
            "include_dirs": [
              "<!(node -p \"require('node-addon-api').include_dir\")"
            ],
            "dependencies": [
              "<!(node -p \"require('node-addon-api').gyp\")"
            ],
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "defines": [
                "UNICODE",
                "_UNICODE"
            ],
            "msvs_settings": {
                "VCCLCompilerTool": {
                    "ExceptionHandling": 1
                }
            }
        }
    ],
    "includes": [
        "auto-top.gypi"
    ]
}
