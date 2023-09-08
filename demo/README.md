# TFLite Micro Model update demo

## Overview
TFLite Micro ML model is a standalone data resource that may requires frequent updates. Most of the changes to the model after the initial deployment are to the weights (and biases) leaving the network architecture and used operators unchanged. This makes it a perfect candidate for OTA update independent from the main firmware saving power, bandwidth and limiting the risk.

In this demo we will:
1) Build an image recognition example with untrained model
2) Run the inference on CS300 model to validate the application failed to make successful image recognition
3) Build the same example with fully trained model
4) Run the firmware from step `1` mapping the model built in step `3` to validate successful image recognitions

## Prebuilt images
In `demo/` directory there is a number of prebuilt binaries that can be used instead of manually building the code.

* `ethos-u-img_class.bad.axf` Full application image, with untrained model
* `ethos-u-img_class.good.axf` Full application image, with fully trained model
* `mobilenet_v2_1.0_224_INT8.bad.tflite` Untrained model file (needs to be compiled)
* `mobilenet_v2_1.0_224_INT8.good.tflite` Fully trained model file (needs to be compiled)
* `model.bad.bin` Binary model image containing untrained model (can be loaded directly with full application image)
* `model.good.bin` Binary model image containing fully trained model (can be loaded directly with full application image)

To run the application on the model you can use the following commands:
```
<path to the model>/FVP_Corstone_SSE-300/models/Linux64_GCC-6.4/FVP_Corstone_SSE-300_Ethos-U55 -C mps3_board.isualisation.disable-visualisation=1 -C cpu0.semihosting-enable=1 -C mps3_board.telnetterminal0.start_telnet=0 -C mps3_board.uart0.out_file="-"  -C mps3_board.uart0.unbuffered_output=1 --stat <full firmware image file>
```

To run the application with updated ML model, add the following option to the full command above:
```
--data <ml model image file>@0x70000000
```

## Building from sources

Use the following guide to setup your environment and fetch the sources: ()[https://review.mlplatform.org/plugins/gitiles/ml/ethos-u/ml-embedded-evaluation-kit/+/HEAD/docs/use_cases/img_class.md].

To build the example with fully trained model follow the guide above.

To build the example with untrained model follow these steps:

1) Remove existing model files from the resources directory:
```
rm resources_downloaded/img_class/mobilenet*
```
2) Copy the untrained model file from `demo/mobilenet_v2_1.0_224_INT8.bad.tflite` to the resources directory:
```
cp demo/mobilenet_v2_1.0_224_INT8.bad.tflite resources_downloaded/img_class/mobilenet_v2_1.0_224_INT8.tflite
```
3) Run the `set_up_default_resources.py` script to run Vela compiler on the model:
```
python3 set_up_default_resources.py
```
4) Configure the build system and build the firmware:
```
mkdir build_img_class && cd build_img_class
cmake ../ -DUSE_CASE_BUILD=img_class
make -j4
```

The full firmware can be found in `build_img_class/bin/ethos-u-img_class.axf` and the model image `build_img_class/bin/sectors/img_class/model.bin`.

The model can also be injected using cmake `img_class_MODEL_TFLITE_PATH` option, but you'll need to compile it by hand in Vela compiler. Please refer to ml-kit documentation how to do it.