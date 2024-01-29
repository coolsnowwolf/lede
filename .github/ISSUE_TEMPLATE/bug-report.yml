name: 问题描述
description: 反馈问题模板
body:
  - type: textarea
    id: description
    attributes:
      label: 详细叙述
      description: 详细叙述问题
    validations:
      required: true
  - type: checkboxes
    id: duplicate_issue
    attributes:
      label: 重复 issue
      description: 是否搜索了 issues
      options:
        - label: 没有类似的 issue
          required: true
  - type: input
    id: model_name
    attributes:
      label: 具体型号
      description: 硬件型号
    validations:
      required: true
  - type: textarea
    id: details_log
    attributes:
      label: 详细日志
      description: make V=s -j1 时的问题日志
    validations:
      required: true
