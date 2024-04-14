#include<bits/stdc++.h>

#define ElemType int

using namespace std;

typedef struct Node{
    ElemType data;
    Node* lchild;
    Node* rchild;

    Node():data(0),lchild(nullptr),rchild(nullptr){}
}*Btree;

// 随机生成一个序列
vector<int> generateRandomSequence(size_t size, int minVal, int maxVal) {
    random_device rd; // 获取随机设备
    mt19937 gen(rd()); // 使用 Mersenne Twister 算法作为随机数引擎
    uniform_int_distribution<int> distribution(minVal, maxVal); // 定义随机数分布

    vector<int> sequence;
    sequence.reserve(size); // 预先分配内存以提高性能

    for (size_t i = 0; i < size; ++i) {
        sequence.push_back(distribution(gen)); // 生成随机数并添加到序列中
    }

    return sequence;
}




// 向二叉树中插入一个节点

//递归创建
// void insert(Btree& root, ElemType val) {
//     if (root == nullptr) {
//         root = new Node;
//         root->data = val;
//     } else {
//         if (val < root->data) {
//             insert(root->lchild, val); // 插入到左子树
//         } else {
//             insert(root->rchild, val); // 插入到右子树
//         }
//     }
// }


//循环创建
void insert(Btree& root, ElemType val) {
    if (root == nullptr) {
        root = new Node;
        root->data = val;
        return;
    }

    Node* curr = root;
    while (true) {
        if (val < curr->data) {
            if (curr->lchild == nullptr) {
                curr->lchild = new Node;
                curr->lchild->data = val;
                return;
            }
            curr = curr->lchild; // 移动到左子节点
        } else {
            if (curr->rchild == nullptr) {
                curr->rchild = new Node;
                curr->rchild->data = val;
                return;
            }
            curr = curr->rchild; // 移动到右子节点
        }
    }
}



void createBtree(Btree& root, const vector<int> arr, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        insert(root, arr[i]);
    }
}


int numOfLeaf(Btree root) {
    if (root == nullptr) {
        return 0;
    }

    int count = 0;
    vector<Node*> stackNodes;
    stackNodes.push_back(root); // 将根节点压入栈中

    while (!stackNodes.empty()) {
        Node* current = stackNodes.back(); // 获取栈顶元素
        stackNodes.pop_back(); // 弹出栈顶元素

        if (current->lchild == nullptr && current->rchild == nullptr) {
            count++; // 叶子节点计数增加
        }

        // 注意先压入右子节点，再压入左子节点
        if (current->rchild != nullptr) {
            stackNodes.push_back(current->rchild);
        }
        if (current->lchild != nullptr) {
            stackNodes.push_back(current->lchild);
        }
    }

    return count;
}

bool isEqual(Btree tree1, Btree tree2) {
    vector<Btree> stack1, stack2;
    stack1.push_back(tree1);
    stack2.push_back(tree2);

    while (!stack1.empty() && !stack2.empty()) {
        Btree node1 = stack1.back();
        Btree node2 = stack2.back();
        stack1.pop_back();
        stack2.pop_back();

        if (node1 == nullptr && node2 == nullptr) {
            continue;
        }

        if (node1 == nullptr || node2 == nullptr || node1->data != node2->data) {
            return false;
        }

        stack1.push_back(node1->rchild);
        stack2.push_back(node2->rchild);

        stack1.push_back(node1->lchild);
        stack2.push_back(node2->lchild);
    }

    return stack1.empty() && stack2.empty();
}


void swapChildren(Btree root) {
    if (root == nullptr) {
        return;
    }

    vector<Btree> stackNodes;
    stackNodes.push_back(root);

    while (!stackNodes.empty()) {
        Btree current = stackNodes.back();
        stackNodes.pop_back();

        // 交换左右孩子
        Btree temp = current->lchild;
        current->lchild = current->rchild;
        current->rchild = temp;

        // 将左右孩子压入栈中
        if (current->lchild != nullptr) {
            stackNodes.push_back(current->lchild);
        }
        if (current->rchild != nullptr) {
            stackNodes.push_back(current->rchild);
        }
    }
}

void preorderTraversal(Btree root) {
    if (root == nullptr) {
        return;
    }

    vector<Btree> stackNodes;
    stackNodes.push_back(root);

    while (!stackNodes.empty()) {
        Btree current = stackNodes.back();
        stackNodes.pop_back();

        cout << current->data << " "; // 输出节点数据

        // 先将右孩子压入栈，再将左孩子压入栈
        if (current->rchild != nullptr) {
            stackNodes.push_back(current->rchild);
        }
        if (current->lchild != nullptr) {
            stackNodes.push_back(current->lchild);
        }
    }
}


int main()
{
    size_t size; // 序列大小
    int minVal = 1; // 序列中随机数的最小值
    int maxVal = 100; // 序列中随机数的最大值

    int num_leaf =0;

    bool flag =true;
    cin >> size;
    vector<int> randomSequence1 = generateRandomSequence(size, minVal, maxVal);
    vector<int> randomSequence2 = generateRandomSequence(size, minVal, maxVal);

    Btree BT1 = new Node;
    Btree BT2 = new Node;
    
    createBtree(BT1,randomSequence1,size);
    createBtree(BT2,randomSequence2,size);

    preorderTraversal(BT1);
    cout << endl;
    preorderTraversal(BT2);
    cout << endl;
    num_leaf = numOfLeaf(BT1);
    
    cout << num_leaf << endl;

    flag = isEqual(BT1,BT2);
    cout << flag << endl;

    swapChildren(BT1);
    preorderTraversal(BT1);
    cout << endl;
    preorderTraversal(BT2);
    
    

    return 0;



}



