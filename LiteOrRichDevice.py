#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (C) 2020. Huawei Technologies Co., Ltd. All rights reserved.

import os
import sys

def main():
    curr_path = sys.argv[0]
    target_path = os.path.abspath(os.path.join(curr_path,
                                    "../../../build/lite/config/component/zlite_component.gni"))
    if (os.path.exists(target_path)):
        res = 1
    else:
        res = 0

    print(res)
    return 0

if __name__ == '__main__':
  sys.exit(main())