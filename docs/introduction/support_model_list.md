# 支持模型

目前，Paddle-Lite已严格验证28个模型的精度和性能。对视觉类模型做到了充分的支持，覆盖分类、检测和定位，也包含了特色的OCR模型的支持。对NLP模型也做到了广泛支持，包含翻译、语义表达等等。

除了已严格验证的模型，Paddle-Lite对其他CV和NLP模型也可以做到大概率支持。

| 类别 | 类别细分 | 模型 | 支持平台 |
|-|-|:-|:-|
| CV | 分类 | [MobileNetV1](https://paddlelite-demo.bj.bcebos.com/models/mobilenet_v1_fp32_224_fluid.tar.gz) | ARM，X86，HuaweiKirinNPU，RKNPU，APU，XPU，HuaweiAscendNPU |
| CV | 分类 | [MobileNetV2](https://paddlelite-demo.bj.bcebos.com/models/mobilenet_v2_fp32_224_fluid.tar.gz) | ARM，X86，HuaweiKirinNPU，XPU，HuaweiAscendNPU |
| CV | 分类 | [ResNet18](https://paddlelite-demo.bj.bcebos.com/models/resnet18_fp32_224_fluid.tar.gz) | ARM，HuaweiKirinNPU，RKNPU，XPU，HuaweiAscendNPU |
| CV | 分类 | [ResNet50](https://paddlelite-demo.bj.bcebos.com/models/resnet50_fp32_224_fluid.tar.gz) | ARM，X86，HuaweiKirinNPU，RKNPU，XPU，HuaweiAscendNPU |
| CV | 分类 | [MnasNet](https://paddlelite-demo.bj.bcebos.com/models/mnasnet_fp32_224_fluid.tar.gz) | ARM，HuaweiKirinNPU，HuaweiAscendNPU |
| CV | 分类 | [EfficientNetB0](https://paddlelite-demo.bj.bcebos.com/models/EfficientNetB0.tar.gz) | ARM，XPU，HuaweiAscendNPU |
| CV | 分类 | [SqueezeNet](https://paddlelite-demo.bj.bcebos.com/models/squeezenet_fp32_224_fluid.tar.gz) | ARM，HuaweiKirinNPU，XPU，HuaweiAscendNPU |
| CV | 分类 | [ShufflenetV2](https://paddlelite-demo.bj.bcebos.com/models/shufflenetv2.tar.gz) | ARM，XPU，HuaweiAscendNPU |
| CV | 分类 | [ShuffleNet](https://paddlepaddle-inference-banchmark.bj.bcebos.com/shufflenet_inference.tar.gz) | ARM |
| CV | 分类 | [InceptionV4](https://paddlelite-demo.bj.bcebos.com/models/inceptionv4.tar.gz) | ARM，X86，HuaweiKirinNPU，XPU，HuaweiAscendNPU |
| CV | 分类 | [VGG16](https://paddlepaddle-inference-banchmark.bj.bcebos.com/VGG16_inference.tar) | ARM，XPU，HuaweiAscendNPU |
| CV | 分类 | [VGG19](https://paddlepaddle-inference-banchmark.bj.bcebos.com/VGG19_inference.tar) | XPU，HuaweiAscendNPU|
| CV | 分类 | [GoogleNet](https://paddlepaddle-inference-banchmark.bj.bcebos.com/GoogleNet_inference.tar) | ARM，X86，XPU |
| CV | 检测 | [MobileNet-SSD](https://paddlelite-demo.bj.bcebos.com/models/ssd_mobilenet_v1_pascalvoc_fp32_300_fluid.tar.gz) | ARM，HuaweiKirinNPU*，HuaweiAscendNPU* |
| CV | 检测 | [YOLOv3-MobileNetV3](https://paddlelite-demo.bj.bcebos.com/models/yolov3_mobilenet_v3_prune86_FPGM_320_fp32_fluid.tar.gz) | ARM，HuaweiKirinNPU*，HuaweiAscendNPU* |
| CV | 检测 | [Faster RCNN](https://paddlepaddle-inference-banchmark.bj.bcebos.com/faster_rcnn.tar) | ARM |
| CV | 检测 | [Mask RCNN*](https://github.com/PaddlePaddle/PaddleDetection/blob/release/0.4/docs/MODEL_ZOO_cn.md) | ARM |
| CV | 分割 | [Deeplabv3](https://paddlelite-demo.bj.bcebos.com/models/deeplab_mobilenet_fp32_fluid.tar.gz) | ARM |
| CV | 分割 | [UNet](https://paddlelite-demo.bj.bcebos.com/models/Unet.zip) | ARM |
| CV | 人脸 | [FaceDetection](https://paddlelite-demo.bj.bcebos.com/models/facedetection_fp32_240_430_fluid.tar.gz) | ARM |
| CV | 人脸 | [FaceBoxes*](https://github.com/PaddlePaddle/PaddleDetection/blob/release/0.4/docs/featured_model/FACE_DETECTION.md#FaceBoxes) | ARM |
| CV | 人脸 | [BlazeFace*](https://github.com/PaddlePaddle/PaddleDetection/blob/release/0.4/docs/featured_model/FACE_DETECTION.md#BlazeFace) | ARM |
| CV | 人脸 | [MTCNN](https://paddlelite-demo.bj.bcebos.com/models/mtcnn.zip) | ARM |
| CV | OCR | [OCR-Attention](https://paddle-inference-dist.bj.bcebos.com/ocr_attention.tar.gz) | ARM |
| CV | GAN | [CycleGAN*](https://github.com/PaddlePaddle/models/tree/release/1.7/PaddleCV/gan/cycle_gan) | HuaweiKirinNPU |
| NLP | 机器翻译 | [Transformer*](https://github.com/PaddlePaddle/models/tree/release/1.8/PaddleNLP/machine_translation/transformer) | ARM，HuaweiKirinNPU* |
| NLP | 机器翻译 | [BERT](https://paddle-inference-dist.bj.bcebos.com/PaddleLite/models_and_data_for_unittests/bert.tar.gz) | XPU |
| NLP | 语义表示 | [ERNIE](https://paddle-inference-dist.bj.bcebos.com/PaddleLite/models_and_data_for_unittests/ernie.tar.gz) | XPU |

**注意：** 
1. 模型列表中 * 代表该模型链接来自[PaddlePaddle/models](https://github.com/PaddlePaddle/models)，否则为推理模型的下载链接
2. 支持平台列表中 HuaweiKirinNPU* 代表ARM+HuaweiKirinNPU异构计算，否则为HuaweiKirinNPU计算
3. 支持平台列表中 HuaweiAscendNPU* 代表X86或ARM+HuaweiAscendNPU异构计算，否则为HuaweiAscendNPU计算
