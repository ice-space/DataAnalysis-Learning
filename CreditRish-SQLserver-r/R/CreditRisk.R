# 基于SQL Service

library(DBI)
library(odbc)

con <- dbConnect(odbc::odbc(), "SQLServiceR")

#########################################################
#                  数据预处理                           #
#########################################################



query <- "SELECT 
    [dbo].[申请客户信息$].*, 
    COALESCE([dbo].[客户信用记录$].审批结果, '1-不通过') AS 审批结果,
    COALESCE([dbo].[客户信用记录$].信用等级, 'E-风险过大客户') AS 信用等级
FROM 
    [dbo].[申请客户信息$]
LEFT JOIN 
    [dbo].[客户信用记录$] 
ON 
    [dbo].[客户信用记录$].[客户号] = [dbo].[申请客户信息$].[客户号];"

result <- dbGetQuery(con, query)

df_result <- data.frame(result)


head(df_result,3)

summary(df_result)

df_clean <- df_result

df_clean$'个人年收入' <- as.numeric(gsub("[^0-9.]", "", df_clean$'个人年收入'))

summary(df_clean)


library(writexl)
write_xlsx(df_clean, "./CustermInfo.xlsx")


#### 清洗数据 #######################################

boxplot(df_clean$'个人年收入', main="Boxplot for Column Name")


df_tranf <- df_clean

cols_to_factor <- c("性别", "教育程度", "职业类别", "户籍", "居住类型", "车辆情况", 
                    "保险缴纳", "信贷情况", "审批结果", "信用等级","婚姻状态")
# 将指定列转换为因子
df_tranf[cols_to_factor] <- lapply(df_tranf[cols_to_factor], as.factor)

str(df_tranf)

summary(df_tranf)
colnames(df_tranf)

# 假设df_tranf是你的数据框
# 列出需要绘制直方图的列
columns_to_plot <- colnames(df_tranf)[!(colnames(df_tranf) %in% c("客户号", "客户姓名", "证件号码"))]

# 设置画布的布局，假设我们要在一个画布上绘制3行5列的子图
par(mfrow = c(3, 5))

# 循环遍历每一列并绘制图表
for (column in columns_to_plot) {
  # 提取列数据
  data <- df_tranf[[column]]
  
  # 判断数据是否为数值型
  if (is.numeric(data)) {
    # 绘制直方图
    hist(data, main = column, xlab = column, col = "skyblue", border = "white")
  } else {
    # 对因子和字符型数据绘制条形图
    barplot(table(data), main = column, xlab = column, col = "skyblue", border = "white")
  }
}

# 重置画布布局
par(mfrow = c(1, 1))



# 使用箱线图检测离群值
boxplot(df_clean$个人年收入, main = "个人年收入的箱线图")

# 使用IQR方法检测和删除离群值
Q1 <- quantile(df_clean$个人年收入, 0.25)
Q3 <- quantile(df_clean$个人年收入, 0.75)
IQR <- Q3 - Q1

# 定义离群值的界限
lower_bound <- Q1 - 1.5 * IQR
upper_bound <- Q3 + 1.5 * IQR

# 找出离群值
outliers <- df_clean$个人年收入 < lower_bound | df_clean$个人年收入 > upper_bound

# 删除离群值
df_clean <- df_clean[!outliers, ]

# 查看删除离群值后的数据摘要
summary(df_clean$个人年收入)

# 使用箱线图检测离群值
boxplot(df_clean$个人年收入, main = "个人年收入的箱线图")





#########################################################
#                  关联规则                            ##
#########################################################
















#########################################################
#                  审核结果                            ##
#########################################################


## 模型构建
df_model <- df_tranf[ , -c(1:3)]
summary(df_model)

colnames(df_model)


### 去除信用等级
df_model <- df_model[, -ncol(df_model)]
summary(df_model)


#检查数据平衡
table(df_model$审批结果)

df_model_z <- df_model
#标准化连续变量
df_model_z$年龄 <- scale(df_model_z$年龄)
df_model_z$工作年限 <- scale(df_model_z$工作年限)
df_model_z$个人年收入 <- scale(df_model_z$个人年收入)

## 划分训练集测试集
set.seed(123)  # 设置随机种子，确保结果可复现
trainIndex <- createDataPartition(df_model_z$审批结果, p = 0.7, list = FALSE)  # target_variable是你的目标变量的列名
training_set <- df_model_z[trainIndex, ]
testing_set <- df_model_z[-trainIndex, ]

library(caret)
library(rpart)
library(rpart.plot)


## 决策树
tree_model <- rpart(审批结果 ~ ., data = training_set, method = "class")
rpart.plot(tree_model)
prp(tree_model)

# 获取特征重要性
importance <- varImp(tree_model)

# 打印特征重要性
print(importance)


## 逻辑回归
# #使用正则化
# library(glmnet)
# 
# # 准备数据
# X <- model.matrix(审批结果 ~ ., data = df_model_z)[, -1]
# y <- df_model_z$审批结果
# 
# # 训练正则化逻辑回归模型
# cvfit <- cv.glmnet(X, y, family = "binomial", alpha = 0.5)  # alpha = 1 为 Lasso, alpha = 0 为 Ridge
# 
# # 查看最佳模型的系数
# coef(cvfit, s = "lambda.min")
# 
# summary(cvfit)

glm_model <- glm(审批结果 ~ ., data = training_set, family = binomial)
summary(glm_model)





# 加载e1071包  
library(e1071) 

# 训练朴素贝叶斯模型
nb_model <- naiveBayes(审批结果 ~ ., data = training_set)

# 查看模型摘要
print(nb_model)

# 预测测试集
nb_predictions <- predict(nb_model, newdata = testing_set)

# 混淆矩阵
confusion_matrix <- confusionMatrix(nb_predictions, testing_set$审批结果)
print(confusion_matrix)

# 查看条件概率
print(nb_model$tables)








## 评价模型
# 训练朴素贝叶斯模型
nb_predictions <- predict(nb_model, newdata = testing_set)
# 混淆矩阵
confusion_matrix <- confusionMatrix(nb_predictions, testing_set$审批结果)
print(confusion_matrix)
## 逻辑回归
# 使用glm_model对测试集进行预测  
glm_predictions <- predict(glm_model, newdata = testing_set, type = "response")  
# 将预测概率转换为类别（例如，使用0.5作为阈值）  
glm_predicted_classes <- ifelse(glm_predictions > 0.5, '1-不通过', '0-通过')  
# 假设testing_set$审批结果是实际的结果变量  
actual_classes <- testing_set$审批结果 
confusion <- confusionMatrix(as.factor(glm_predicted_classes), as.factor(actual_classes))  
print(confusion)
## 决策树
tree_predictions <- predict(tree_model, newdata = testing_set, type = "class")
confusionMatrix(tree_predictions, testing_set$审批结果)




library(dplyr) 
## 删除无关变量后重新拟合
df_model_z_cleaned <- df_model_z %>%  
  select(-c(婚姻状态, 户籍, 性别))  

# 查看处理后的数据框  
head(df_model_z_cleaned)

## 划分训练集测试集
set.seed(123)  # 设置随机种子，确保结果可复现
trainIndex <- createDataPartition(df_model_z_cleaned$审批结果, p = 0.7, list = FALSE)  # target_variable是你的目标变量的列名
training_set <- df_model_z_cleaned[trainIndex, ]
testing_set <- df_model_z_cleaned[-trainIndex, ]

## 决策树
tree_model <- rpart(审批结果 ~ ., data = training_set, method = "class")
rpart.plot(tree_model)
prp(tree_model)

# 获取特征重要性
importance <- varImp(tree_model)

# 打印特征重要性
print(importance)

tree_predictions <- predict(tree_model, newdata = testing_set, type = "class")
confusionMatrix(tree_predictions, testing_set$审批结果)














#########################################################
#                  信用等级                            ##
#########################################################
## 模型构建
df_model <- df_tranf[ , -c(1:3)]
summary(df_model)

colnames(df_model)


### 去除审批结果
library(dplyr)
df_model <- select(df_model, -审批结果)
summary(df_model)

colnames(df_model)
#检查数据平衡
table(df_model$信用等级)

df_model_z <- df_model
#标准化连续变量
df_model_z$年龄 <- scale(df_model_z$年龄)
df_model_z$工作年限 <- scale(df_model_z$工作年限)
df_model_z$个人年收入 <- scale(df_model_z$个人年收入)



library(caret)
library(rpart)
library(rpart.plot)


## 划分训练集测试集
set.seed(123)  # 设置随机种子，确保结果可复现
trainIndex <- createDataPartition(df_model_z$信用等级, p = 0.7, list = FALSE)  # target_variable是你的目标变量的列名
training_set <- df_model_z[trainIndex, ]
testing_set <- df_model_z[-trainIndex, ]

# 查看新的训练集类别分布
table(training_set$信用等级)

# # 使用 SMOTE 进行过采样
# library(smotefamily)
# smote_data <- SMOTE(training_set[ , -ncol(training_set)], training_set$信用等级)
# training_set_smote <- smote_data$data
# training_set_smote$信用等级 <- as.factor(training_set_smote$class)
# training_set_smote$class <- NULL

# ## 决策树
# tree_model <- rpart(信用等级 ~ ., data = training_set, method = "class")
# rpart.plot(tree_model)
# prp(tree_model)
# 
# # 获取特征重要性
# importance <- varImp(tree_model)
# varImp(tree_model)
# # 打印特征重要性
# print(importance)



glm_model <- glm(信用等级 ~ ., data = training_set, family = binomial)
summary(glm_model)







library(randomForest)
# 随机森林模型训练


# 随机森林模型训练
set.seed(123)
p <- ncol(training_set) - 1  # 特征数
mtry_value <- floor(sqrt(p))



rf_model <- randomForest(信用等级 ~ ., data = training_set, ntree = 100, mtry = mtry_value, importance = TRUE)
# 查看模型
print(rf_model)
# 预测测试集
rf_predictions <- predict(rf_model, testing_set)
# 模型性能评估
confusionMatrix(rf_predictions, testing_set$信用等级)
# 重要特征可视化
importance <- importance(rf_model)
varImpPlot(rf_model)
print(importance)



