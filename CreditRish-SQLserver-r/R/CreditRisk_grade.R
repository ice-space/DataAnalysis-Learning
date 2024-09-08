library(DBI)
library(odbc)

con <- dbConnect(odbc::odbc(), "SQLServiceR")

query <- "SELECT [dbo].[客户信用记录$].*,[dbo].[申请客户信息$].个人年收入,[dbo].[申请客户信息$].年龄,[dbo].[申请客户信息$].信贷情况
FROM [dbo].[客户信用记录$] 
LEFT JOIN [dbo].[申请客户信息$] 
ON [dbo].[客户信用记录$].[客户号] = [dbo].[申请客户信息$].[客户号];"

result <- dbGetQuery(con, query)

df_result <- data.frame(result)

## 数据预处理
summary(df_result)
df_clean <- df_result

df_clean$'个人年收入' <- as.numeric(gsub("[^0-9.]", "", df_clean$'个人年收入'))


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

# 转化因子变量
df_tranf <- df_clean

colnames(df_clean)

cols_to_factor <- c("性别", "教育程度", "职业类别", "户籍", "居住类型", "车辆情况", 
                    "保险缴纳", "信贷情况", "审批结果", "信用等级","婚姻状态")
# 将指定列转换为因子
df_tranf[cols_to_factor] <- lapply(df_tranf[cols_to_factor], as.factor)

str(df_tranf)

summary(df_tranf)
colnames(df_tranf)



# 选择需要保留的列作为结果列
df_x <- df_tranf[, c("客户号", "客户姓名", "性别", "年龄_连续", "婚姻状态", "户籍", "教育程度", 
                          "居住类型", "职业类别", "工作年限", "个人收入_连续", "保险缴纳", "车辆情况",
                          "个人年收入", "年龄", "信贷情况")]

# 提取并移除作为结果列的变量
df_y <- df_tranf[, c("客户号", "信用总评分", "信用等级", "额度")]

# 检查一下数据是否符合预期
summary(df_x)
summary(df_y)

# 添加信用总评分列到 df_x 中
df_x_xy <- merge(df_x, df_y[, c("客户号", "信用总评分")], by = "客户号", all.x = TRUE)

# 确认添加后的数据框
summary(df_x_xy)


# 选择需要进行独热编码的列
cols_to_encode <- c("性别", "婚姻状态", "户籍", "教育程度", "居住类型", "职业类别", "保险缴纳", "车辆情况", "信贷情况")

# 执行独热编码
df_encoded <- df_x_xy

# 将需要编码的列转换为因子以确保正确编码
df_encoded[cols_to_encode] <- lapply(df_encoded[cols_to_encode], as.factor)

# 使用 model.matrix 进行独热编码，同时排除截距项 (-1)
df_encoded <- model.matrix(~ . - 1, data = df_encoded[, cols_to_encode]) %>%
  as.data.frame()

# 将编码后的列与原始数据合并
df_encoded <- cbind(df_x_xy[, -which(names(df_x_xy) %in% cols_to_encode)], df_encoded)

summary(df_encoded)


df_encoded <- df_encoded[,-c(1,2)]

summary(df_encoded)


# 确认数据结构和列名
colnames(df_encoded)

# 线性回归建模
lm_model <- lm(信用总评分 ~ ., data = df_encoded)

# 查看线性回归模型摘要
summary(lm_model)



# 添加信用总评分列到 df_x 中
df_x_xydj <- merge(df_x, df_y[, c("客户号", "信用等级")], by = "客户号", all.x = TRUE)

# 确认添加后的数据框
summary(df_x_xydj)

df_model_xydj <- df_x_xydj[,-c(1,2)]
summary(df_model_xydj)

# 逻辑回归分析
glm_model <- glm(信用等级 ~ ., data = df_model_xydj, family = "binomial")

# 查看逻辑回归模型摘要
summary(glm_model)



# 决策树
set.seed(123)  # 设置随机种子，确保结果可复现
trainIndex <- createDataPartition(df_model_xydj$信用等级, p = 0.7, list = FALSE)  # target_variable是你的目标变量的列名
training_set <- df_model_xydj[trainIndex, ]
testing_set <- df_model_xydj[-trainIndex, ]

tree_model <- rpart(信用等级 ~ ., data = training_set, method = "class")
rpart.plot(tree_model)
prp(tree_model)

predictions <- predict(tree_model, newdata = testing_set, type = "class")
confusionMatrix(predictions, testing_set$信用等级)

# 获取特征重要性
importance <- varImp(tree_model)

# 打印特征重要性
print(importance)

