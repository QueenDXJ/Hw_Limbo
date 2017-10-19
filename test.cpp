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
void inordertravel(Bitree T){ //Ϊʲô���ﲻ����&T ��Ϊû�ж�T�����Ķ��� 
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
	sort(a,a+100); //�鵽�ĺ��� ��̫��Ϊʲôa,a+100 
	Bitree root = NULL;
	int begin=0;
	int end=100;
	BuildTree(root,a,begin,end);
	inordertravel(root);
	getchar();
	//��֪��Ϊʲô������г������и�0_(:�١���)_ 
	return 0;
}
