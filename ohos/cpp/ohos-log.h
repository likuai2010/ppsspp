//
// Created on 2024/6/14.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef VULKANOHOS_HILOG_H
#define VULKANOHOS_HILOG_H

// 解决日志冲突
namespace OHOS{
    #include <hilog/log.h>
};

#endif //VULKANOHOS_HILOG_H
