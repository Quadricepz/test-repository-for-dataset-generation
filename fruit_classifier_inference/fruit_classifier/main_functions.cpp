/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include "config.h"
#include "constants.h"
#include "main_functions.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "model_data.h"

// RGB Sensor imports
#include "rgb_sensor/Adafruit_TCS34725.h"
#include "rgb_sensor/setup.h"
#include "rgb_sensor/TCS34725_utils.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
int inference_count = 0;

constexpr int kTensorArenaSize = 100000;
uint8_t tensor_arena[kTensorArenaSize];
}  // namespace



// The name of this function is important for Arduino compatibility.
void initialize_all() {
  
  // initialize RGB Sensor
  setup();

  // Initialize Tensorflow lite mirco
  tflite::InitializeTarget();

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(quantized_model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  // This pulls in all the operation implementations we need.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroMutableOpResolver<4> resolver;
  TfLiteStatus resolve_status = resolver.AddFullyConnected();
  if (resolve_status != kTfLiteOk) {
    MicroPrintf("Op resolution failed for FullyConnected");
    return;
  }
  resolve_status = resolver.AddRelu();
  if (resolve_status != kTfLiteOk) {
    MicroPrintf("Op resolution failed for Relu");
    return;
  }
  resolve_status = resolver.AddSoftmax();
  if (resolve_status != kTfLiteOk) {
    MicroPrintf("Op resolution failed for Softmax");
    return;
  }

  resolve_status = resolver.AddQuantize(); 
  if (resolve_status != kTfLiteOk) {
      MicroPrintf("Op resolution failed for Quantize");
      return;
  }
  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    return;
  }

  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);
  output = interpreter->output(0);

  // Keep track of how many inferences we have performed.
  inference_count = 0;
}

// The name of this function is important for Arduino compatibility.
void loop() {

  // Get the Sensor RGB Data
  rgb_sensor.getData();
  // Quantize the inputs from uint16_t to uint8_t
  uint8_t redChannelQuantized   = static_cast<uint8_t>(rgb_sensor.r_comp / 256);
  uint8_t greenChannelQuantized = static_cast<uint8_t>(rgb_sensor.g_comp / 256);
  uint8_t blueChannelQuantized  = static_cast<uint8_t>(rgb_sensor.b_comp / 256);

  // Place the quantized inputs in the model's input tensor
  input->data.uint8[0] = redChannelQuantized;
  input->data.uint8[1] = greenChannelQuantized;
  input->data.uint8[2] = blueChannelQuantized;

  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    MicroPrintf("Invoke failed on x1: %f, x2: %f, x3: %f\n", static_cast<double>(redChannelQuantized), static_cast<double>(greenChannelQuantized), static_cast<double>(blueChannelQuantized));
    return;
  }

  // Obtain the quantized outputs from model's output tensor
  int8_t y1_quantized = output->data.int8[0];
  int8_t y2_quantized = output->data.int8[1];
  int8_t y3_quantized = output->data.int8[2];

  // Dequantize the outputs from integer to floating-point
  float y1 = (y1_quantized - output->params.zero_point) * output->params.scale;
  float y2 = (y2_quantized - output->params.zero_point) * output->params.scale;
  float y3 = (y3_quantized - output->params.zero_point) * output->params.scale;

  MicroPrintf("Model outputs: %s: %f, %s: %f, %s: %f \n", CLASSNAME0, y1, CLASSNAME1, y2, CLASSNAME2, y3);

  // Increment the inference_counter, and reset it if we have reached
  // the total number per cycle
  inference_count += 1;
  if (inference_count >= kInferencesPerCycle) inference_count = 0;
}

