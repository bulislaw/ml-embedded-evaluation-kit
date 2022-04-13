/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
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
#ifndef IMG_CLASS_PROCESSING_HPP
#define IMG_CLASS_PROCESSING_HPP

#include "BaseProcessing.hpp"
#include "Model.hpp"
#include "Classifier.hpp"

namespace arm {
namespace app {

    /**
     * @brief   Pre-processing class for Image Classification use case.
     *          Implements methods declared by BasePreProcess and anything else needed
     *          to populate input tensors ready for inference.
     */
    class ImgClassPreProcess : public BasePreProcess {

    public:
        /**
         * @brief       Constructor
         * @param[in]   model   Pointer to the the Image classification Model object.
         **/
        explicit ImgClassPreProcess(Model* model);

        /**
         * @brief       Should perform pre-processing of 'raw' input image data and load it into
         *              TFLite Micro input tensors ready for inference
         * @param[in]   input      Pointer to the data that pre-processing will work on.
         * @param[in]   inputSize  Size of the input data.
         * @return      true if successful, false otherwise.
         **/
        bool DoPreProcess(const void* input, size_t inputSize) override;
    };

    /**
     * @brief   Post-processing class for Image Classification use case.
     *          Implements methods declared by BasePostProcess and anything else needed
     *          to populate result vector.
     */
    class ImgClassPostProcess : public BasePostProcess {

    private:
        Classifier& m_imgClassifier;
        const std::vector<std::string>& m_labels;
        std::vector<ClassificationResult>& m_results;

    public:
        /**
         * @brief       Constructor
         * @param[in]   classifier   Classifier object used to get top N results from classification.
         * @param[in]   model        Pointer to the the Image classification Model object.
         * @param[in]   labels       Vector of string labels to identify each output of the model.
         * @param[in]   results      Vector of classification results to store decoded outputs.
         **/
        ImgClassPostProcess(Classifier& classifier, Model* model,
                            const std::vector<std::string>& labels,
                            std::vector<ClassificationResult>& results);

        /**
         * @brief       Should perform post-processing of the result of inference then populate
         *              populate classification result data for any later use.
         * @return      true if successful, false otherwise.
         **/
        bool DoPostProcess() override;
    };

} /* namespace app */
} /* namespace arm */

#endif /* IMG_CLASS_PROCESSING_HPP */