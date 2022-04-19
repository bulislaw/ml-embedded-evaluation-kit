/*
 * Copyright (c) 2021-2022 Arm Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "UseCaseHandler.hpp"

#include "Classifier.hpp"
#include "InputFiles.hpp"
#include "MobileNetModel.hpp"
#include "ImageUtils.hpp"
#include "UseCaseCommonUtils.hpp"
#include "hal.h"
#include "log_macros.h"
#include "ImgClassProcessing.hpp"

#include <cinttypes>

using ImgClassClassifier = arm::app::Classifier;

namespace arm {
namespace app {

    /* Image classification inference handler. */
    bool ClassifyImageHandler(ApplicationContext& ctx, uint32_t imgIndex, bool runAll)
    {
        auto& profiler = ctx.Get<Profiler&>("profiler");
        auto& model = ctx.Get<Model&>("model");
        /* If the request has a valid size, set the image index as it might not be set. */
        if (imgIndex < NUMBER_OF_FILES) {
            if (!SetAppCtxIfmIdx(ctx, imgIndex, "imgIndex")) {
                return false;
            }
        }
        auto initialImgIdx = ctx.Get<uint32_t>("imgIndex");

        constexpr uint32_t dataPsnImgDownscaleFactor = 2;
        constexpr uint32_t dataPsnImgStartX = 10;
        constexpr uint32_t dataPsnImgStartY = 35;

        constexpr uint32_t dataPsnTxtInfStartX = 150;
        constexpr uint32_t dataPsnTxtInfStartY = 40;


        if (!model.IsInited()) {
            printf_err("Model is not initialised! Terminating processing.\n");
            return false;
        }

        TfLiteTensor* inputTensor = model.GetInputTensor(0);
        if (!inputTensor->dims) {
            printf_err("Invalid input tensor dims\n");
            return false;
        } else if (inputTensor->dims->size < 4) {
            printf_err("Input tensor dimension should be = 4\n");
            return false;
        }

        /* Get input shape for displaying the image. */
        TfLiteIntArray* inputShape = model.GetInputShape(0);
        const uint32_t nCols = inputShape->data[arm::app::MobileNetModel::ms_inputColsIdx];
        const uint32_t nRows = inputShape->data[arm::app::MobileNetModel::ms_inputRowsIdx];
        const uint32_t nChannels = inputShape->data[arm::app::MobileNetModel::ms_inputChannelsIdx];

        /* Set up pre and post-processing. */
        ImgClassPreProcess preprocess = ImgClassPreProcess(&model);

        std::vector<ClassificationResult> results;
        ImgClassPostProcess postprocess = ImgClassPostProcess(ctx.Get<ImgClassClassifier&>("classifier"), &model,
                ctx.Get<std::vector<std::string>&>("labels"), results);

        UseCaseRunner runner = UseCaseRunner(&preprocess, &postprocess, &model);

        do {
            hal_lcd_clear(COLOR_BLACK);

            /* Strings for presentation/logging. */
            std::string str_inf{"Running inference... "};

            const uint8_t* imgSrc = get_img_array(ctx.Get<uint32_t>("imgIndex"));
            if (nullptr == imgSrc) {
                printf_err("Failed to get image index %" PRIu32 " (max: %u)\n", ctx.Get<uint32_t>("imgIndex"),
                           NUMBER_OF_FILES - 1);
                return false;
            }

            /* Display this image on the LCD. */
            hal_lcd_display_image(
                imgSrc,
                nCols, nRows, nChannels,
                dataPsnImgStartX, dataPsnImgStartY, dataPsnImgDownscaleFactor);

            /* Display message on the LCD - inference running. */
            hal_lcd_display_text(str_inf.c_str(), str_inf.size(),
                    dataPsnTxtInfStartX, dataPsnTxtInfStartY, false);

            /* Select the image to run inference with. */
            info("Running inference on image %" PRIu32 " => %s\n", ctx.Get<uint32_t>("imgIndex"),
                get_filename(ctx.Get<uint32_t>("imgIndex")));

            const size_t imgSz = inputTensor->bytes < IMAGE_DATA_SIZE ?
                                  inputTensor->bytes : IMAGE_DATA_SIZE;

            /* Run the pre-processing, inference and post-processing. */
            if (!runner.PreProcess(imgSrc, imgSz)) {
                return false;
            }

            profiler.StartProfiling("Inference");
            if (!runner.RunInference()) {
                return false;
            }
            profiler.StopProfiling();

            if (!runner.PostProcess()) {
                return false;
            }

            /* Erase. */
            str_inf = std::string(str_inf.size(), ' ');
            hal_lcd_display_text(str_inf.c_str(), str_inf.size(),
                    dataPsnTxtInfStartX, dataPsnTxtInfStartY, false);

            /* Add results to context for access outside handler. */
            ctx.Set<std::vector<ClassificationResult>>("results", results);

#if VERIFY_TEST_OUTPUT
            TfLiteTensor* outputTensor = model.GetOutputTensor(0);
            arm::app::DumpTensor(outputTensor);
#endif /* VERIFY_TEST_OUTPUT */

            if (!PresentInferenceResult(results)) {
                return false;
            }

            profiler.PrintProfilingResult();

            IncrementAppCtxIfmIdx(ctx,"imgIndex");

        } while (runAll && ctx.Get<uint32_t>("imgIndex") != initialImgIdx);

        return true;
    }

} /* namespace app */
} /* namespace arm */
