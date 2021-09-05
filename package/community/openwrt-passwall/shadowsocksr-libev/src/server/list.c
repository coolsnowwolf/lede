#include "list.h"

/// 文件：list_impl.c
/// 功能：实现链表的基本操作
/// 作者：bluewind
/// 完成时间：2011.5.29
/// 修改时间：2011.5.31, 2011.7.2
/// 修改备注：在头节点处添加一个空节点，可以优化添加、删除节点代码
///  再次修改，链表增加节点数据data_size，限制数据大小，修改了
///  添加复制数据代码，修正重复添加节点后释放节点的Bug，添加了前
///  插、排序和遍历功能，7.3 添加tail尾指针，改进后插法性能，并改名
/// --------------------------------------------------------------

void swap_data(Node n1, Node n2);

/// --------------------------------------------------------------
//  函数名：list_init
//  功能：  链表初始化
//  参数：  无
//  返回值：已初始化链表指针
//  备注：  链表本身动态分配，由list_destroy函数管理释放
/// --------------------------------------------------------------
List list_init(unsigned int data_size)
{
    List list = (List) malloc(sizeof(struct clist));
    if(list != NULL)                                        //内存分配成功
    {
        list->head = (Node) malloc(sizeof(node));           //为头节点分配内存
        if(list->head)          //内存分配成功
        {
            list->head->data = NULL;      //初始化头节点
            list->head->next = NULL;
            list->data_size  = data_size;
            list->tail = list->head;
            list->size = 0;

            list->add_back  = list_add_back;   //初始化成员函数
            list->add_front  = list_add_front;
            list->delete_node = list_delete_node;
            list->delete_at  = list_delete_at;
            list->modify_at  = list_modify_at;
            list->have_same  = list_have_same;
            list->have_same_cmp  = list_have_same_cmp;
            list->foreach  = list_foreach;
            list->clear   = list_clear;
            list->sort   = list_sort;
            list->destroy  = list_destroy;
        }
    }
    return list;
}

/// --------------------------------------------------------------
//  函数名：list_add_back
//  功能：  添加链表结点 (后插法)
//  参数：  l--链表指针，data--链表数据指针，可为任意类型
//  返回值：int型，为1表示添加成功，为0表示添加失败
//  备注：  如果链表本身为空或是分配节点内存失败，将返回0
/// --------------------------------------------------------------
int  list_add_back(List l, void *data)
{
    Node new_node = (Node) malloc(sizeof(node));

    if(l != NULL && new_node != NULL)  //链表本身不为空，且内存申请成功
    {
        new_node->data = malloc(l->data_size);
        memcpy(new_node->data, data, l->data_size);
        new_node->next = NULL;

        l->tail->next = new_node;   //添加节点
        l->tail = new_node;     //记录尾节点位置
        l->size ++;       //链表元素总数加1

        return 1;
    }

    return 0;
}

/// --------------------------------------------------------------
//  函数名：list_add_front
//  功能：  添加链表结点 (前插法)
//  参数：  l--链表指针，data--链表数据指针，可为任意类型
//  返回值：int型，为1表示添加成功，为0表示添加失败
//  备注：  如果链表本身为空或是分配节点内存失败，将返回0
/// --------------------------------------------------------------
int list_add_front(List l, void *data)
{
    Node new_node = (Node) malloc(sizeof(node));

    if(l != NULL && new_node != NULL)
    {
        new_node->data = malloc(l->data_size);
        memcpy(new_node->data, data, l->data_size);
        new_node->next = l->head->next;

        l->head->next = new_node;
        if(!l->size)        //记录尾指针位置
            l->tail = new_node;
        l->size ++;

        return 1;
    }

    return 0;
}

/// --------------------------------------------------------------
//  函数名：list_delete_node
//  功能：删除链表结点
//  参数：l--链表指针，data--链表数据指针，可为任意类型
//        *pfunc为指向一个数据类型比较的函数指针
//  返回值：int型，为1表示删除成功，为0表示没有找到匹配数据
//  备注：*pfunc函数接口参数ndata为节点数据，data为比较数据，返回为真表示匹配数据
/// --------------------------------------------------------------
int  list_delete_node(List l, void *data, int (*pfunc)(void *ndata, void *data))
{
    if(l != NULL)
    {
        Node prev = l->head;      //前一个节点
        Node curr = l->head->next;     //当前节点

        while(curr != NULL)
        {
            if(pfunc(curr->data, data))    //如果找到匹配数据
            {
                if(curr == l->tail)     //如果是删除尾节点
                 l->tail = prev;

                prev->next = prev->next->next;  //修改前节点next指针指向下下个节点

                free(curr->data);     //释放节点数据
                free(curr);       //释放节点

                l->size--;       //链表元素总数减1
                return 1;       //返回真值
            }
            prev = prev->next;      //没有找到匹配时移动前节点和当前节点
            curr = curr->next;
        }
    }

 return 0;         //没有找到匹配数据
}

/// --------------------------------------------------------------
//  函数名：list_delete_at
//  功能：  修改链表节点元素值
//  参数：  l--链表指针，index--索引值, 范围(0 -- size-1)
//  返回值：int型，为1表示删除成功，为0表示删除失败
//  备注：  如果链表本身为空或是index为非法值，将返回0
/// --------------------------------------------------------------
int list_delete_at(List l, unsigned int index)
{
    unsigned int cindex = 0;

    if(l != NULL && index >= 0 && index < l->size)
    {
        Node prev = l->head;      //前一个节点
        Node curr = l->head->next;     //当前节点

        while(cindex != index)
        {
            prev = prev->next;
            curr = curr->next;
            cindex ++;
        }

        if(index == (l->size) - 1)
            l->tail = prev;

        prev->next = prev->next->next;
        free(curr->data);
        free(curr);
        l->size --;

        return 1;
    }

    return 0;
}

/// --------------------------------------------------------------
//  函数名：list_modify_at
//  功能：  修改链表节点元素值
//  参数：  l--链表指针，index--索引值, 范围(0 -- size-1)
//   data--链表数据指针
//  返回值：int型，为1表示修改成功，为0表示修改失败
//  备注：  如果链表本身为空或是index为非法值，将返回0
/// --------------------------------------------------------------
int list_modify_at(List l, unsigned int index, void *new_data)
{
    unsigned int cindex = 0;

    if(l != NULL && index >= 0 && index < l->size )  //非空链表，并且index值合法
    {
        Node curr = l->head->next;
        while(cindex != index)
        {
            curr = curr->next;
            cindex ++;
        }
        memcpy(curr->data, new_data, l->data_size);
        return 1;
    }

    return 0;
}

/// --------------------------------------------------------------
//  函数名：list_sort
//  功能：  链表排序
//  参数：  l--链表指针，*pfunc为指向一个数据类型比较的函数指针
//  返回值：无
//  备注：  使用简单选择排序法，相比冒泡法每次交换，效率高一点
/// --------------------------------------------------------------
void list_sort(List l, compare pfunc)
{
    if(l != NULL)
    {
        Node min, icurr, jcurr;

        icurr = l->head->next;
        while(icurr)
        {
            min = icurr;        //记录最小值
            jcurr = icurr->next;      //内循环指向下一个节点
            while(jcurr)
            {
                if(pfunc(min->data, jcurr->data))  //如果找到n+1到最后一个元素最小值
                    min = jcurr;      //记录下最小值的位置

                jcurr = jcurr->next;
            }

            if(min != icurr)       //当最小值位置和n+1元素位置不相同时
            {
                swap_data(min, icurr);     //才进行交换，减少交换次数
            }

            icurr = icurr->next;
        }
    }
}

void swap_data(Node n1, Node n2)
{
    void *temp;

    temp = n2->data;
    n2->data = n1->data;
    n1->data = temp;
}


int list_have_same(List l, void *data, int (*pfunc)(void *ndata, void *data))
{
    if(l != NULL)
    {
        Node curr;

        for(curr = l->head->next; curr != NULL; curr = curr->next)
        {
            if(pfunc(curr->data, data))
            {
                return 1;
            }
        }
    }

    return 0;
}

int list_have_same_cmp(List l, void *data)
{
    if(l != NULL)
    {
        Node curr;

        for(curr = l->head->next; curr != NULL; curr = curr->next)
        {
            if(memcmp(curr->data, data, l->data_size))
            {
                return 1;
            }
        }
    }

    return 0;
}

/// --------------------------------------------------------------
//  函数名：list_foreach
//  功能：  遍历链表元素
//  参数：  l--链表指针，doit为指向一个处理数据的函数指针
//  返回值：无
//  备注：  doit申明为void (*dofunc)(void *ndata)原型
/// --------------------------------------------------------------
void list_foreach(List l, dofunc doit)
{
    if(l != NULL)
    {
        Node curr;

        for(curr = l->head->next; curr != NULL; curr = curr->next)
        {
            doit(curr->data);
        }
    }
}

/// --------------------------------------------------------------
//  函数名：list_clear
//  功能：  清空链表元素
//  参数：  l--链表指针
//  返回值：无
// 备注： 没有使用先Destroy再Init链表的办法，直接实现
/// --------------------------------------------------------------
void list_clear(List l)
{
    if(l != NULL)
    {
        Node temp;
        Node curr = l->head->next;

        while(curr != NULL)
        {
            temp = curr->next;

            free(curr->data);    //释放节点和数据
            free(curr);

            curr = temp;
        }

        l->size = 0;      //重置链表数据
        l->head->next = NULL;
        l->tail = l->head;
    }
}

/// --------------------------------------------------------------
//  函数名：list_destroy
//  功能：  释放链表
//  参数：  l--链表指针
//  返回值：空链表指针
/// --------------------------------------------------------------
List list_destroy(List l)
{
    if(l != NULL)
    {
        Node temp;

        while(l->head)
        {
            temp = l->head->next;

            if(l->head->data != NULL)   //如果是头节点就不释放数据空间
            free(l->head->data);   //先释放节点数据(但是节点数据里也有指针？)
            free(l->head);      //再释放节点

            l->head = temp;
        }

        free(l);        //释放链表本身占用空间
        l = NULL;
    }

    return l;
}
