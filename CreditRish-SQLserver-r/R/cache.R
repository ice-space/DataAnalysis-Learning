# 设置类别变量

freq_table <- table(df_clean$性别)
categories <- names(freq_table)
cat("类别数量:", length(categories), "\n")
cat("类别值:\n")
print(categories)

df_tranf <- df_tranf %>%
  mutate(性别 = case_when(
    性别 == "男" ~ 1L,
    性别 == "女" ~ 2L,
    TRUE ~ NA_real_
  ))


freq_table <- table(df_clean$婚姻状态)
categories <- names(freq_table)
cat("类别数量:", length(categories), "\n")
cat("类别值:\n")
print(categories)


df_tranf <- df_tranf %>%
  mutate(婚姻状态 = case_when(
    婚姻状态 == "未婚" ~ 0L,
    婚姻状态 == "已婚" ~ 1L,
    婚姻状态 == "丧偶" ~ 2L,
    婚姻状态 == "离异" ~ 3L,
    TRUE ~ NA_real_
  ))

freq_table <- table(df_clean$教育程度)
categories <- names(freq_table)
cat("类别数量:", length(categories), "\n")
cat("类别值:\n")
print(categories)

df_tranf <- df_tranf %>%
  mutate(教育程度 = case_when(
    教育程度 == "初中及以下" ~ 0L,
    教育程度 == "高中" ~ 1L,
    教育程度 == "大专" ~ 2L,
    教育程度 == "本科" ~ 3L,
    教育程度 == "硕士及以上" ~ 4L,
    TRUE ~ NA_real_
  ))

freq_table <- table(df_clean$保险缴纳)
categories <- names(freq_table)
cat("类别数量:", length(categories), "\n")
cat("类别值:\n")
print(categories)


df_tranf <- df_tranf %>%
  mutate(保险缴纳 = case_when(
    保险缴纳 == "无" ~ 0L,
    保险缴纳 == "有" ~ 1L,
    TRUE ~ NA_real_
  ))


freq_table <- table(df_clean$职业类别)
categories <- names(freq_table)
cat("类别数量:", length(categories), "\n")
cat("类别值:\n")
print(categories)


df_tranf <- df_tranf %>%
  mutate(职业类别 = case_when(
    职业类别 == "个体户" ~ 0L,
    职业类别 == "国有企业" ~ 1L,
    职业类别 == "其他企业" ~ 2L,
    职业类别 == "私营企业" ~ 3L,
    职业类别 == "外资企业" ~ 4L,
    TRUE ~ NA_real_
  ))

freq_table <- table(df_clean$户籍)
categories <- names(freq_table)
cat("类别数量:", length(categories), "\n")
cat("类别值:\n")
print(categories)













## 独热编码
# 进行独热编码
df_model_encoded <- df_model %>%
  mutate(审批结果 = as.factor(审批结果)) %>%
  select(-审批结果) %>%
  model.matrix(~ . - 1, data = .) %>%
  as.data.frame()

# 加入目标变量
df_model_encoded$审批结果 <- df_model$审批结果

# 加载必要的包
library(e1071)

# 设置随机种子，确保结果可复现
set.seed(123)

# 划分训练集和测试集
trainIndex <- createDataPartition(df_model_encoded$审批结果, p = 0.7, list = FALSE)
training_set <- df_model_encoded[trainIndex, ]
testing_set <- df_model_encoded[-trainIndex, ]

# 训练SVM模型
svm_model <- svm(审批结果 ~ ., data = training_set)

# 查看模型摘要
summary(svm_model)

# 预测
predictions <- predict(svm_model, newdata = testing_set)