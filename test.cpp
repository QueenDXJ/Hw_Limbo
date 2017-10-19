#include <iostream>
#include<queue>
#include<windows.h>
#include<algorithm>

using namespace std;

typedef struct node{
	int val;
	struct node *left;
	struct node *right;	
}Node,*Bitree;

void BuildTree(Bitree &T,int a[],int begin,int end){
	if(begin>end){
		return;
	}	
	int mid=begin+(end-begin)/2;
	if(T==NULL){
		T=(Node *)malloc(sizeof(Node));// learn from the Internet
		T->val=a[mid];
		T->left=NULL;
		T->right=NULL;
	}
	BuildTree(T->left,a,begin,mid-1);
	BuildTree(T->right,a,mid+1,end);
}
void inordertravel(Bitree T){ //为什么这里不是用&T 因为没有对T做出改动吗？ 
	if(T!=NULL){
		inordertravel(T->left);
		cout<<T->val<<" ";// inorder
		inordertravel(T->right);
	}
}
int main(){
	int a[100];
	for(int i=0;i<100;i++){
		a[i]=100-i;
		cout<<a[i]<<" ";
	}
	cout<<endl;
	sort(a,a+100); //查到的函数 不太懂为什么a,a+100 
	Bitree root = NULL;
	int begin=0;
	int end=100;
	BuildTree(root,a,begin,end);
	inordertravel(root);
	getchar();
	//不知道为什么最后运行出来还有个0_(:з」∠)_ 
	return 0;
}
