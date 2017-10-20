#include <windows.h>
#include <stdio.h>
#include <time.h>

#define CELL 20
#define ROWS 30
#define COLS 25
//升级所需分数值
#define SCORE_LEVEL_INC 80
#define ID_TIMER 1

/////////////////全局变量/////////////////////////////
HWND hwnd;     //保存窗口句柄

int score=0;    //分数
int level=0;    //级数
int interval_unit=25;  //随级数递增的时间间隔增量
int interval_base=300;  //时间间隔基量
int old_interval;   //保存当前的时间间隔，用于加速操作

int cur_left,cur_top;  //记录方块当前的位置
int width_block,height_block; //方块的宽带和高度

bool isPause=false;    //暂停标识
UINT timer_id=0;    //保存计时器ID

static byte *block=NULL;  //方块，方块为随机大小，采用动态分配内存方式，所以这里是指针变量
byte g_panel[ROWS][COLS]={0};
////////////////////////////////////////////////////
LRESULT CALLBACK WndProc ( HWND,UINT,WPARAM,LPARAM );
void DrawPanel ( HDC hdc );  //绘制表格
void RefreshPanel ( HDC hdc );  //刷新面板
void DoDownShift ( HDC hdc );  //下移
void DoLeftShift ( HDC hdc );  //左移
void DoRightShift ( HDC hdc );  //右移
void DoAccelerate ( HDC hdc );  //加速
void DoRedirection ( HDC hdc ); //改变方向
void ClearRow ( HDC hdc );   //消行
bool ExportBlock();  //输出方块，
//该函数会直接修改全局变量block,width_block,height_block,
//cur_left和cur_top
bool IsTouchBottom ( HDC hdc );   //判断是否到达底部

void color(int a)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),a);
}
int main()
{
 HINSTANCE hInstance=GetModuleHandle ( NULL );
 TCHAR szAppName[]=TEXT ( "teris" );
 MSG msg;
 WNDCLASS wc;
 wc.style=CS_HREDRAW|CS_VREDRAW;
 wc.lpfnWndProc=WndProc;
 wc.cbClsExtra=0;
 wc.cbWndExtra=0;
 wc.hInstance=hInstance;
 wc.hIcon=LoadIcon ( NULL,IDI_APPLICATION );
 wc.hCursor=LoadCursor ( NULL,IDC_ARROW );
 wc.hbrBackground= ( HBRUSH ) GetStockObject ( WHITE_BRUSH );
 wc.lpszMenuName=NULL;
 wc.lpszClassName=szAppName;
 if ( !RegisterClass ( &wc ) )
 {
  printf ( "RegisterClass occur errors!" );
  return 0;
 }
 hwnd=CreateWindow ( szAppName,TEXT ( "-------- 俄罗斯方块 --------" ),
                     WS_OVERLAPPEDWINDOW,
                     0,0,0,0,
                     NULL,
                     NULL,
                     hInstance,
                     NULL );
 ShowWindow ( hwnd,SW_SHOW );
 UpdateWindow ( hwnd );
 while ( GetMessage ( &msg,NULL,0,0 ) )
 {
  TranslateMessage ( &msg );
  DispatchMessage ( &msg );
 }
 return msg.wParam;
}

void DrawPanel ( HDC hdc )    //绘制游戏面板
{
    color(13);
 int x,y;
 RECT rect;

 for ( y=0; y<ROWS; y++ )
 {
  for ( x=0; x<COLS; x++ )
  {
   //计算方块的边框范围
   rect.top=y*CELL+1;
   rect.bottom= ( y+1 ) *CELL-1;
   rect.left=x*CELL+1;
   rect.right= ( x+1 ) *CELL-1;
   FrameRect ( hdc,&rect, ( HBRUSH ) GetStockObject ( BLACK_BRUSH ) );
  }
 }
}

void DoDownShift ( HDC hdc )    //下移
{
 if ( NULL==block ) return;

 //判断是否到达底部
 if ( IsTouchBottom ( hdc ) )   //到底部
 {
  //消行处理
  ClearRow ( hdc );
  ExportBlock();  //输出下一个方块
 }

 cur_top++;
 RefreshPanel ( hdc );
}

void DoLeftShift ( HDC hdc )    //左移
{
 int x,y;
 if ( NULL==block ) return;

 if ( 0==cur_left ) return;
 if ( cur_top<0 ) return; //方块没有完整显示前，不能左移
 for ( y=0; y<height_block; y++ )
 {
  for ( x=0; x<width_block; x++ )    //从左边开始扫描，获取该行最左边的实心方格块
  {
   if ( * ( block+y*width_block+x ) )
   {
    //判断当前方格在面板上面左边一个方格是否为实心，是就代表不能再左移
    if ( g_panel[cur_top+y][cur_left+x-1] ) return;

    break;  //只判断最左边的一个实心方格，之后直接扫描下一行
   }
  }
 }
 cur_left--;
 RefreshPanel ( hdc );
}

void DoRightShift ( HDC hdc )    //右移
{
 int x,y;
 if ( NULL==block ) return;

 if ( COLS-width_block==cur_left ) return;
 if ( cur_top<0 ) return;  //方块完整显示前不能右移
 for ( y=0; y<height_block; y++ )
 {
  for ( x=width_block-1; x>=0; x-- )   //从右边开始扫描，获取该行最右边的实心方格块
  {
   if ( * ( block+y*width_block+x ) )
   {
    //判断当前方格在面板上右边一个方格是否为实心，是就代表不能再右移
    if ( g_panel[cur_top+y][cur_left+x+1] ) return;

    break;  //只判断最右边的一个实心方格
   }
  }
 }
 cur_left++;
 RefreshPanel ( hdc );
}

void DoRedirection ( HDC hdc )   //改变方向
{
 int i,j;
 byte * temp=NULL;
 if ( NULL==block ) return;
 if ( cur_top<0 ) return;  //方块完整显示前不能转向

 temp= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
 for ( i=0; i<width_block; i++ )
 {
  for ( j=0; j<height_block; j++ )
  {
   //temp[i][j]=block[height_block-j-1][i];
   * ( temp+i*height_block+j ) =* ( block+ ( height_block-j-1 ) *width_block+i );
  }
 }

 //给方块重新定位
 int incHeight=width_block-height_block;
 int incWidth=height_block-width_block;
 int temp_cur_top=cur_top-incHeight/2;
 int temp_cur_left=cur_left-incWidth/2;

 //system("cls");
 //printf("temp_top=%d, temp_left=%d",temp_cur_top,temp_cur_left);

 //判断当前空间是否足够让方块改变方向
 int max_len=max ( width_block,height_block );
 //防止下标访问越界
 if ( temp_cur_top+max_len-1>=ROWS||temp_cur_left<0||temp_cur_left+max_len-1>=COLS )
 {
  free ( temp );  //退出前必须先释放内存
  return;
 }
 for ( i=0; i<max_len; i++ )
 {
  for ( j=0; j<max_len; j++ )
  {
   //转向所需的空间内有已被占用的实心方格存在，即转向失败
   if ( g_panel[temp_cur_top+i][temp_cur_left+j] )
   {
    free ( temp );  //退出前必须先释放内存
    return;
   }
  }
 }

 //把临时变量的值赋给block，只能赋值，而不能赋指针值
 for ( i=0; i<height_block; i++ )
 {
  for ( j=0; j<width_block; j++ )
  {
   //block[i][j]=temp[i][j];
   * ( block+i*width_block+j ) =* ( temp+i*width_block+j );
  }
 }

 //全局变量重新被赋值
 cur_top=temp_cur_top;
 cur_left=temp_cur_left;
 //交换
 i=width_block;
 width_block=height_block;
 height_block=i;

 free ( temp );  //释放为临时变量分配的内存
 RefreshPanel ( hdc );
}

void DoAccelerate ( HDC hdc )    //加速
{
 if ( NULL==block ) return;

 if ( IsTouchBottom ( hdc ) )
 {
  //消行处理
  ClearRow ( hdc );
  ExportBlock();
 }
 cur_top++;
 RefreshPanel ( hdc );
}

bool IsTouchBottom ( HDC hdc )
{
 int x,y;
 int i,j;

 if ( NULL==block ) return false;
 if ( ROWS==cur_top+height_block )
 {
  //固定方块
  for ( i=0; i<height_block; i++ )
  {
   for ( j=0; j<width_block; j++ )
   {
    if ( * ( block+i*width_block+j ) ) g_panel[cur_top+i][cur_left+j]=1;
   }
  }
  return true;
 }
 for ( y=height_block-1; y>=0; y-- )    //从底行开始扫描
 {
  //判断第一个实心方块在面板上邻接的下方方格是否为实心，是就代表已经到达底部
  for ( x=0; x<width_block; x++ )
  {
   if ( * ( block+y*width_block+x ) )
   {
    if ( cur_top+y+1<0 ) return false;
    if ( g_panel[cur_top+y+1][cur_left+x] )
    {
     //判断是否gameover
     if ( cur_top<=0 )
     {
      if ( timer_id )
      {
       KillTimer ( hwnd,ID_TIMER );
       timer_id=0;
      }
      MessageBox ( hwnd,TEXT ( "游戏结束" ),TEXT ( "游戏到此结束！" ),MB_OK|MB_ICONEXCLAMATION );
      SendMessage ( hwnd,WM_CLOSE,0,0 );
     }
     //
     //固定方块
     for ( i=0; i<height_block; i++ )
     {
      for ( j=0; j<width_block; j++ )
      {
       if ( * ( block+i*width_block+j ) ) g_panel[cur_top+i][cur_left+j]=1;
      }
     }
     return true;
    }
   }
  }
 }
 return false;
}

void ClearRow ( HDC hdc )      //消行
{
 int i,j,k;
 int count=0;  //消行次数
 bool isFilled;
 //消行处理
 for ( i=ROWS-1; i>=0; i-- )
 {
  isFilled=true;
  for ( j=0; j<COLS; j++ )
  {
   if ( !g_panel[i][j] )
   {
    isFilled=false;
    break;
   }
  }
  if ( isFilled )
  {
   for ( j=0; j<COLS; j++ )
   {
    g_panel[i][j]=0;
   }
   //所有方块往下移
   for ( k=i-1; k>=0; k-- )
   {
    for ( j=0; j<COLS; j++ )
    {
     g_panel[k+1][j]=g_panel[k][j];
    }
   }
   i=i+1;
   count++;
  }
 }

 //最高级别为9级，所以分数极限为(9+1)*SCORE_LEVEL_INC-1
 if ( score>=10*SCORE_LEVEL_INC-1 ) return;

 //加分规则：消除行数，1行加10分，2行加15分,3行加20分,4行加30分
 switch ( count )
 {
 case 1:
  score+=10;
  break;
 case 2:
  score+=15;
  break;
 case 3:
  score+=20;
  break;
 case 4:
  score+=30;
  break;
 }

 int temp_level=score/SCORE_LEVEL_INC;
 if ( temp_level>level )
 {
  level=temp_level;
  //撤销当前计时器，然后重设
  if ( timer_id ) KillTimer ( hwnd,ID_TIMER );
  timer_id=SetTimer ( hwnd,ID_TIMER,interval_base-level*interval_unit,NULL );
 }

 system ( "cls" );
 printf ( "score: %d, level: %d ",score,level );
}

void RefreshPanel ( HDC hdc )     //刷新面板
{
 int x,y;
 RECT rect;
 HBRUSH h_bSolid= ( HBRUSH ) GetStockObject ( GRAY_BRUSH ),
                  h_bEmpty= ( HBRUSH ) GetStockObject ( WHITE_BRUSH );
 if ( NULL==block ) return;

 //先刷屏
 for ( y=0; y<ROWS; y++ )
 {
  for ( x=0; x<COLS; x++ )
  {
   //为避免刷掉方块的边框，rect范围必须比边框范围小1
   rect.top=y*CELL+2;
   rect.bottom= ( y+1 ) *CELL-2;
   rect.left=x*CELL+2;
   rect.right= ( x+1 ) *CELL-2;
   if ( g_panel[y][x] )
    FillRect ( hdc,&rect,h_bSolid );
   else
    FillRect ( hdc,&rect,h_bEmpty );
  }
 }
 //再定位方块
 for ( y=0; y<height_block; y++ )
 {
  for ( x=0; x<width_block; x++ )
  {
   if ( * ( block+y*width_block+x ) )    //实心
   {
    rect.top= ( y+cur_top ) *CELL+2;
    rect.bottom= ( y+cur_top+1 ) *CELL-2;
    rect.left= ( x+cur_left ) *CELL+2;
    rect.right= ( x+cur_left+1 ) *CELL-2;
    FillRect ( hdc,&rect,h_bSolid );
   }
  }
 }
}

bool ExportBlock()    //输出方块
{
 int sel;
  color(12);
 if ( block )
 {
  free ( block );  //释放之前分配的内存
  block=NULL;
 }

 sel=rand() %7;
 switch ( sel )
 {
 case 0:  //水平条
  width_block=4;
  height_block=1;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =1;  //可以理解为*(block+0*width_block+0)=1，即第一行的第一个方格，下面同理
  * ( block+1 ) =1;  //*(block+0*width_block+1)=1
  * ( block+2 ) =1;  //*(block+0*width_block+2)=1
  * ( block+3 ) =1;  //*(block+0*width_block+3)=1

  cur_top=0-height_block;
  cur_left= ( COLS-width_block ) /2;
  break;
 case 1:  //三角
      color(13);
  width_block=3;
  height_block=2;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =0;  //可以理解为*(block+0*width_block+0)=0，即第一行的第一个方格，下面同理
  * ( block+1 ) =1;  //*(block+0*width_block+1)=1
  * ( block+2 ) =0;  //*(block+0*width_block+2)=0
  * ( block+3 ) =1;  //*(block+1*width_block+0)=1，第二行开始
  * ( block+4 ) =1;  //*(block+1*width_block+1)=1
  * ( block+5 ) =1;  //*(block+1*width_block+2)=1

  cur_top=0-height_block;
  cur_left= ( COLS-width_block ) /2;
  break;
 case 2:  //左横折
      color(10);
  width_block=3;
  height_block=2;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =1;  //可以理解为*(block+0*width_block+0)=1，下面同理
  * ( block+1 ) =0;  //*(block+0*width_block+1)=0
  * ( block+2 ) =0;  //*(block+0*width_block+2)=0
  * ( block+3 ) =1;  //*(block+1*width_block+0)=1
  * ( block+4 ) =1;  //*(block+1*width_block+1)=1
  * ( block+5 ) =1;  //*(block+1*width_block+2)=1

  cur_top=0-height_block;
  cur_left= ( COLS-width_block ) /2;
  break;
 case 3:  //右横折
      color(9);
  width_block=3;
  height_block=2;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =0;  //可以理解为*(block+0*width_block+0)=0，下面同理
  * ( block+1 ) =0;  //*(block+0*width_block+1)=0
  * ( block+2 ) =1;  //*(block+0*width_block+2)=1
  * ( block+3 ) =1;  //*(block+1*width_block+0)=1
  * ( block+4 ) =1;  //*(block+1*width_block+1)=1
  * ( block+5 ) =1;  //*(block+1*width_block+2)=1

  cur_top=0-height_block;
  cur_left= ( COLS-width_block ) /2;
  break;
 case 4:  //左闪电
      color(9);
  width_block=3;
  height_block=2;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =1;  //可以理解为*(block+0*width_block+0)=1，下面同理
  * ( block+1 ) =1;  //*(block+0*width_block+1)=1
  * ( block+2 ) =0;  //*(block+0*width_block+2)=0
  * ( block+3 ) =0;  //*(block+1*width_block+0)=0
  * ( block+4 ) =1;  //*(block+1*width_block+1)=1
  * ( block+5 ) =1;  //*(block+1*width_block+2)=1

  cur_top=0-height_block;
  cur_left= ( COLS-width_block ) /2;
  break;
 case 5:  //右闪电
      color(8);
  width_block=3;
  height_block=2;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =0;  //可以理解为*(block+0*width_block+0)=0，下面同理
  * ( block+1 ) =1;  //*(block+0*width_block+1)=1
  * ( block+2 ) =1;  //*(block+0*width_block+2)=1
  * ( block+3 ) =1;  //*(block+1*width_block+0)=1
  * ( block+4 ) =1;  //*(block+1*width_block+1)=1
  * ( block+5 ) =0;  //*(block+1*width_block+2)=0

  cur_top=0-height_block;
  cur_left= ( COLS-width_block ) /2;
  break;
 case 6:  //石头
      color(13);
  width_block=2;
  height_block=2;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =1;  //可以理解为*(block+0*width_block+0)=1，下面同理
  * ( block+1 ) =1;  //*(block+0*width_block+1)=1
  * ( block+2 ) =1;  //*(block+1*width_block+0)=1
  * ( block+3 ) =1;  //*(block+1*width_block+1)=1

  cur_top=0-height_block;
  cur_left= ( COLS-width_block ) /2;
  break;
 }
 return block!=NULL;
}

LRESULT CALLBACK WndProc ( HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam )
{
 HDC hdc;
 PAINTSTRUCT ps;
 //TCHAR szBuffer[1024];

 switch ( message )
 {
 case WM_CREATE:
  MoveWindow ( hwnd,400,10,CELL*COLS+8,CELL*ROWS+32,FALSE );  //补齐宽度和高度
  srand ( time ( NULL ) );
  ExportBlock();

  timer_id=SetTimer ( hwnd,ID_TIMER,interval_base-level*interval_unit,NULL );
  return 0;
 case WM_TIMER:
  hdc=GetDC ( hwnd );
  DoDownShift ( hdc );
  ReleaseDC ( hwnd,hdc );
  return 0;
 case WM_KEYDOWN:
  hdc=GetDC ( hwnd );
  switch ( wParam )
  {
  case VK_LEFT:       //左移
   if ( !isPause ) DoLeftShift ( hdc );
   break;
  case VK_RIGHT:       //右移
   if ( !isPause ) DoRightShift ( hdc );
   break;
  case VK_UP:        //转向
   if ( !isPause ) DoRedirection ( hdc );
   break;
  case VK_DOWN:       //加速
   if ( !isPause ) DoAccelerate ( hdc );
   break;
  case VK_SPACE:  //暂停
   isPause=!isPause;
   if ( isPause )
   {
    if ( timer_id ) KillTimer ( hwnd,ID_TIMER );
    timer_id=0;
   }
   else
   {
    timer_id=SetTimer ( hwnd,ID_TIMER,interval_base-level*interval_unit,FALSE );
   }
   break;
  }
  ReleaseDC ( hwnd,hdc );
  return 0;
 case WM_PAINT:
  hdc=BeginPaint ( hwnd,&ps );
  DrawPanel ( hdc );   //绘制面板
  RefreshPanel ( hdc );  //刷新
  EndPaint ( hwnd,&ps );
  return 0;
 case WM_DESTROY:
  if ( block ) free ( block );
  if ( timer_id ) KillTimer ( hwnd,ID_TIMER );
  PostQuitMessage ( 0 );
  return 0;
 }
 return DefWindowProc ( hwnd,message,wParam,lParam );
}
