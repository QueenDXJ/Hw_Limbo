#include <windows.h>
#include <stdio.h>
#include <time.h>

#define CELL 20
#define ROWS 30
#define COLS 25
//�����������ֵ
#define SCORE_LEVEL_INC 80
#define ID_TIMER 1

/////////////////ȫ�ֱ���/////////////////////////////
HWND hwnd;     //���洰�ھ��

int score=0;    //����
int level=0;    //����
int interval_unit=25;  //�漶��������ʱ��������
int interval_base=300;  //ʱ��������
int old_interval;   //���浱ǰ��ʱ���������ڼ��ٲ���

int cur_left,cur_top;  //��¼���鵱ǰ��λ��
int width_block,height_block; //����Ŀ���͸߶�

bool isPause=false;    //��ͣ��ʶ
UINT timer_id=0;    //�����ʱ��ID

static byte *block=NULL;  //���飬����Ϊ�����С�����ö�̬�����ڴ淽ʽ������������ָ�����
byte g_panel[ROWS][COLS]={0};
////////////////////////////////////////////////////
LRESULT CALLBACK WndProc ( HWND,UINT,WPARAM,LPARAM );
void DrawPanel ( HDC hdc );  //���Ʊ��
void RefreshPanel ( HDC hdc );  //ˢ�����
void DoDownShift ( HDC hdc );  //����
void DoLeftShift ( HDC hdc );  //����
void DoRightShift ( HDC hdc );  //����
void DoAccelerate ( HDC hdc );  //����
void DoRedirection ( HDC hdc ); //�ı䷽��
void ClearRow ( HDC hdc );   //����
bool ExportBlock();  //������飬
//�ú�����ֱ���޸�ȫ�ֱ���block,width_block,height_block,
//cur_left��cur_top
bool IsTouchBottom ( HDC hdc );   //�ж��Ƿ񵽴�ײ�

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
 hwnd=CreateWindow ( szAppName,TEXT ( "-------- ����˹���� --------" ),
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

void DrawPanel ( HDC hdc )    //������Ϸ���
{
    color(13);
 int x,y;
 RECT rect;

 for ( y=0; y<ROWS; y++ )
 {
  for ( x=0; x<COLS; x++ )
  {
   //���㷽��ı߿�Χ
   rect.top=y*CELL+1;
   rect.bottom= ( y+1 ) *CELL-1;
   rect.left=x*CELL+1;
   rect.right= ( x+1 ) *CELL-1;
   FrameRect ( hdc,&rect, ( HBRUSH ) GetStockObject ( BLACK_BRUSH ) );
  }
 }
}

void DoDownShift ( HDC hdc )    //����
{
 if ( NULL==block ) return;

 //�ж��Ƿ񵽴�ײ�
 if ( IsTouchBottom ( hdc ) )   //���ײ�
 {
  //���д���
  ClearRow ( hdc );
  ExportBlock();  //�����һ������
 }

 cur_top++;
 RefreshPanel ( hdc );
}

void DoLeftShift ( HDC hdc )    //����
{
 int x,y;
 if ( NULL==block ) return;

 if ( 0==cur_left ) return;
 if ( cur_top<0 ) return; //����û��������ʾǰ����������
 for ( y=0; y<height_block; y++ )
 {
  for ( x=0; x<width_block; x++ )    //����߿�ʼɨ�裬��ȡ��������ߵ�ʵ�ķ����
  {
   if ( * ( block+y*width_block+x ) )
   {
    //�жϵ�ǰ����������������һ�������Ƿ�Ϊʵ�ģ��Ǿʹ�����������
    if ( g_panel[cur_top+y][cur_left+x-1] ) return;

    break;  //ֻ�ж�����ߵ�һ��ʵ�ķ���֮��ֱ��ɨ����һ��
   }
  }
 }
 cur_left--;
 RefreshPanel ( hdc );
}

void DoRightShift ( HDC hdc )    //����
{
 int x,y;
 if ( NULL==block ) return;

 if ( COLS-width_block==cur_left ) return;
 if ( cur_top<0 ) return;  //����������ʾǰ��������
 for ( y=0; y<height_block; y++ )
 {
  for ( x=width_block-1; x>=0; x-- )   //���ұ߿�ʼɨ�裬��ȡ�������ұߵ�ʵ�ķ����
  {
   if ( * ( block+y*width_block+x ) )
   {
    //�жϵ�ǰ������������ұ�һ�������Ƿ�Ϊʵ�ģ��Ǿʹ�����������
    if ( g_panel[cur_top+y][cur_left+x+1] ) return;

    break;  //ֻ�ж����ұߵ�һ��ʵ�ķ���
   }
  }
 }
 cur_left++;
 RefreshPanel ( hdc );
}

void DoRedirection ( HDC hdc )   //�ı䷽��
{
 int i,j;
 byte * temp=NULL;
 if ( NULL==block ) return;
 if ( cur_top<0 ) return;  //����������ʾǰ����ת��

 temp= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
 for ( i=0; i<width_block; i++ )
 {
  for ( j=0; j<height_block; j++ )
  {
   //temp[i][j]=block[height_block-j-1][i];
   * ( temp+i*height_block+j ) =* ( block+ ( height_block-j-1 ) *width_block+i );
  }
 }

 //���������¶�λ
 int incHeight=width_block-height_block;
 int incWidth=height_block-width_block;
 int temp_cur_top=cur_top-incHeight/2;
 int temp_cur_left=cur_left-incWidth/2;

 //system("cls");
 //printf("temp_top=%d, temp_left=%d",temp_cur_top,temp_cur_left);

 //�жϵ�ǰ�ռ��Ƿ��㹻�÷���ı䷽��
 int max_len=max ( width_block,height_block );
 //��ֹ�±����Խ��
 if ( temp_cur_top+max_len-1>=ROWS||temp_cur_left<0||temp_cur_left+max_len-1>=COLS )
 {
  free ( temp );  //�˳�ǰ�������ͷ��ڴ�
  return;
 }
 for ( i=0; i<max_len; i++ )
 {
  for ( j=0; j<max_len; j++ )
  {
   //ת������Ŀռ������ѱ�ռ�õ�ʵ�ķ�����ڣ���ת��ʧ��
   if ( g_panel[temp_cur_top+i][temp_cur_left+j] )
   {
    free ( temp );  //�˳�ǰ�������ͷ��ڴ�
    return;
   }
  }
 }

 //����ʱ������ֵ����block��ֻ�ܸ�ֵ�������ܸ�ָ��ֵ
 for ( i=0; i<height_block; i++ )
 {
  for ( j=0; j<width_block; j++ )
  {
   //block[i][j]=temp[i][j];
   * ( block+i*width_block+j ) =* ( temp+i*width_block+j );
  }
 }

 //ȫ�ֱ������±���ֵ
 cur_top=temp_cur_top;
 cur_left=temp_cur_left;
 //����
 i=width_block;
 width_block=height_block;
 height_block=i;

 free ( temp );  //�ͷ�Ϊ��ʱ����������ڴ�
 RefreshPanel ( hdc );
}

void DoAccelerate ( HDC hdc )    //����
{
 if ( NULL==block ) return;

 if ( IsTouchBottom ( hdc ) )
 {
  //���д���
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
  //�̶�����
  for ( i=0; i<height_block; i++ )
  {
   for ( j=0; j<width_block; j++ )
   {
    if ( * ( block+i*width_block+j ) ) g_panel[cur_top+i][cur_left+j]=1;
   }
  }
  return true;
 }
 for ( y=height_block-1; y>=0; y-- )    //�ӵ��п�ʼɨ��
 {
  //�жϵ�һ��ʵ�ķ�����������ڽӵ��·������Ƿ�Ϊʵ�ģ��Ǿʹ����Ѿ�����ײ�
  for ( x=0; x<width_block; x++ )
  {
   if ( * ( block+y*width_block+x ) )
   {
    if ( cur_top+y+1<0 ) return false;
    if ( g_panel[cur_top+y+1][cur_left+x] )
    {
     //�ж��Ƿ�gameover
     if ( cur_top<=0 )
     {
      if ( timer_id )
      {
       KillTimer ( hwnd,ID_TIMER );
       timer_id=0;
      }
      MessageBox ( hwnd,TEXT ( "��Ϸ����" ),TEXT ( "��Ϸ���˽�����" ),MB_OK|MB_ICONEXCLAMATION );
      SendMessage ( hwnd,WM_CLOSE,0,0 );
     }
     //
     //�̶�����
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

void ClearRow ( HDC hdc )      //����
{
 int i,j,k;
 int count=0;  //���д���
 bool isFilled;
 //���д���
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
   //���з���������
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

 //��߼���Ϊ9�������Է�������Ϊ(9+1)*SCORE_LEVEL_INC-1
 if ( score>=10*SCORE_LEVEL_INC-1 ) return;

 //�ӷֹ�������������1�м�10�֣�2�м�15��,3�м�20��,4�м�30��
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
  //������ǰ��ʱ����Ȼ������
  if ( timer_id ) KillTimer ( hwnd,ID_TIMER );
  timer_id=SetTimer ( hwnd,ID_TIMER,interval_base-level*interval_unit,NULL );
 }

 system ( "cls" );
 printf ( "score: %d, level: %d ",score,level );
}

void RefreshPanel ( HDC hdc )     //ˢ�����
{
 int x,y;
 RECT rect;
 HBRUSH h_bSolid= ( HBRUSH ) GetStockObject ( GRAY_BRUSH ),
                  h_bEmpty= ( HBRUSH ) GetStockObject ( WHITE_BRUSH );
 if ( NULL==block ) return;

 //��ˢ��
 for ( y=0; y<ROWS; y++ )
 {
  for ( x=0; x<COLS; x++ )
  {
   //Ϊ����ˢ������ı߿�rect��Χ����ȱ߿�ΧС1
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
 //�ٶ�λ����
 for ( y=0; y<height_block; y++ )
 {
  for ( x=0; x<width_block; x++ )
  {
   if ( * ( block+y*width_block+x ) )    //ʵ��
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

bool ExportBlock()    //�������
{
 int sel;
  color(12);
 if ( block )
 {
  free ( block );  //�ͷ�֮ǰ������ڴ�
  block=NULL;
 }

 sel=rand() %7;
 switch ( sel )
 {
 case 0:  //ˮƽ��
  width_block=4;
  height_block=1;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =1;  //�������Ϊ*(block+0*width_block+0)=1������һ�еĵ�һ����������ͬ��
  * ( block+1 ) =1;  //*(block+0*width_block+1)=1
  * ( block+2 ) =1;  //*(block+0*width_block+2)=1
  * ( block+3 ) =1;  //*(block+0*width_block+3)=1

  cur_top=0-height_block;
  cur_left= ( COLS-width_block ) /2;
  break;
 case 1:  //����
      color(13);
  width_block=3;
  height_block=2;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =0;  //�������Ϊ*(block+0*width_block+0)=0������һ�еĵ�һ����������ͬ��
  * ( block+1 ) =1;  //*(block+0*width_block+1)=1
  * ( block+2 ) =0;  //*(block+0*width_block+2)=0
  * ( block+3 ) =1;  //*(block+1*width_block+0)=1���ڶ��п�ʼ
  * ( block+4 ) =1;  //*(block+1*width_block+1)=1
  * ( block+5 ) =1;  //*(block+1*width_block+2)=1

  cur_top=0-height_block;
  cur_left= ( COLS-width_block ) /2;
  break;
 case 2:  //�����
      color(10);
  width_block=3;
  height_block=2;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =1;  //�������Ϊ*(block+0*width_block+0)=1������ͬ��
  * ( block+1 ) =0;  //*(block+0*width_block+1)=0
  * ( block+2 ) =0;  //*(block+0*width_block+2)=0
  * ( block+3 ) =1;  //*(block+1*width_block+0)=1
  * ( block+4 ) =1;  //*(block+1*width_block+1)=1
  * ( block+5 ) =1;  //*(block+1*width_block+2)=1

  cur_top=0-height_block;
  cur_left= ( COLS-width_block ) /2;
  break;
 case 3:  //�Һ���
      color(9);
  width_block=3;
  height_block=2;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =0;  //�������Ϊ*(block+0*width_block+0)=0������ͬ��
  * ( block+1 ) =0;  //*(block+0*width_block+1)=0
  * ( block+2 ) =1;  //*(block+0*width_block+2)=1
  * ( block+3 ) =1;  //*(block+1*width_block+0)=1
  * ( block+4 ) =1;  //*(block+1*width_block+1)=1
  * ( block+5 ) =1;  //*(block+1*width_block+2)=1

  cur_top=0-height_block;
  cur_left= ( COLS-width_block ) /2;
  break;
 case 4:  //������
      color(9);
  width_block=3;
  height_block=2;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =1;  //�������Ϊ*(block+0*width_block+0)=1������ͬ��
  * ( block+1 ) =1;  //*(block+0*width_block+1)=1
  * ( block+2 ) =0;  //*(block+0*width_block+2)=0
  * ( block+3 ) =0;  //*(block+1*width_block+0)=0
  * ( block+4 ) =1;  //*(block+1*width_block+1)=1
  * ( block+5 ) =1;  //*(block+1*width_block+2)=1

  cur_top=0-height_block;
  cur_left= ( COLS-width_block ) /2;
  break;
 case 5:  //������
      color(8);
  width_block=3;
  height_block=2;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =0;  //�������Ϊ*(block+0*width_block+0)=0������ͬ��
  * ( block+1 ) =1;  //*(block+0*width_block+1)=1
  * ( block+2 ) =1;  //*(block+0*width_block+2)=1
  * ( block+3 ) =1;  //*(block+1*width_block+0)=1
  * ( block+4 ) =1;  //*(block+1*width_block+1)=1
  * ( block+5 ) =0;  //*(block+1*width_block+2)=0

  cur_top=0-height_block;
  cur_left= ( COLS-width_block ) /2;
  break;
 case 6:  //ʯͷ
      color(13);
  width_block=2;
  height_block=2;
  block= ( byte * ) malloc ( sizeof ( byte ) *width_block*height_block );
  * ( block+0 ) =1;  //�������Ϊ*(block+0*width_block+0)=1������ͬ��
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
  MoveWindow ( hwnd,400,10,CELL*COLS+8,CELL*ROWS+32,FALSE );  //�����Ⱥ͸߶�
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
  case VK_LEFT:       //����
   if ( !isPause ) DoLeftShift ( hdc );
   break;
  case VK_RIGHT:       //����
   if ( !isPause ) DoRightShift ( hdc );
   break;
  case VK_UP:        //ת��
   if ( !isPause ) DoRedirection ( hdc );
   break;
  case VK_DOWN:       //����
   if ( !isPause ) DoAccelerate ( hdc );
   break;
  case VK_SPACE:  //��ͣ
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
  DrawPanel ( hdc );   //�������
  RefreshPanel ( hdc );  //ˢ��
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
