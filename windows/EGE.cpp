#include<iostream>
#include<graphics.h>
#include<ege/sys_edit.h> 
using namespace std;
class initGraphics{
private:
	sys_edit* pinput;
	mouse_msg mouse;
	MUSIC music;
	PIMAGE img;
	PIMAGE pictu[10];
	int Width;
    int Height;
    struct Bottom{
    	int xl;
    	int xh;
    	int yl;
    	int yh;
    	bool flag;
	}bottom[10];
public:
	initGraphics(int width=640,int height=480,int inputNum=1,bool flag=true)
	{
		if(width>0&&height>0)
		{
			Width=width;
			Height=height;
		}
		else
		{
			Width=640;
			Height=480;
		}
		if(flag)
			initgraph(width,height,INIT_DEFAULT|INIT_RENDERMANUAL);
		else
			initgraph(Width,Height,0);
		memset(bottom,0,sizeof(Bottom)*10);
		setrendermode(RENDER_AUTO);
		img=newimage();
		for(int i=0;i<10;i++)
			pictu[i]=newimage();
		if(inputNum>0)
			pinput=new sys_edit[inputNum];
	}
	~initGraphics()
	{
		delimage(img);
		delete[] pinput;
		for(int i=0;i<10;i++)
			delimage(pictu[i]);
		closegraph();
	}
	bool musicPlay(const char* ps)
	{
		music.OpenFile(ps);
		if(music.IsOpen())
			music.Play();
		else
			return false;
		return true;
	}
	inline void musicStop()
	{
		music.Pause();
	}
	inline void musicContinue()
	{
		music.Play();
	}
	void changeMusic(const char* ps)
	{
		music.Stop();
		music.Close();
		this->musicPlay(ps);
	}
	void createInput(int num,int x,int y,int sizeX,int sizeY,color_t bk,color_t word,int wordh,int wordw,char* pbuf,int lenbuf,int maxlen=50,bool flag=false,const char* ptext="ËÎÌå")
	{
		flushkey();
		pinput[num].create(flag);
		pinput[num].move(x,y);
		pinput[num].size(sizeX+10,sizeY+10);
		pinput[num].setmaxlen(maxlen);
		pinput[num].visible(true);
		pinput[num].setbgcolor(bk);
		pinput[num].setcolor(word);
		pinput[num].setfont(wordh,wordw,ptext);
		pinput[num].setfocus();
		while(1)
		{
			if(keystate(key_enter))
			{
				pinput[num].gettext(lenbuf,pbuf);
				break;
			}
		}
		pinput[num].destroy();
		flushkey();
	}
	void createInputBottom(int bottomNum,int num,int x,int y,int sizeX,int sizeY,color_t bk,color_t word,int wordh,int wordw,char* pbuf,int lenbuf,int maxlen=50,bool if_bottom=false,bool flag=false,const char* ptext="ËÎÌå")
	{
		int temp=-1;
		flushkey();
		pinput[num].create(flag);
		pinput[num].move(x,y);
		pinput[num].size(sizeX+10,sizeY+10);
		pinput[num].setmaxlen(maxlen);
		pinput[num].visible(true);
		pinput[num].setbgcolor(bk);
		pinput[num].setcolor(word);
		pinput[num].setfont(wordh,wordw,ptext);
		pinput[num].setfocus();
		while(if_bottom)
		{
			temp=this->waitMsgBottom();
			if(temp==bottomNum)
				break;
		}
		if(if_bottom)
		{
			pinput[num].gettext(lenbuf,pbuf);
			pinput[num].destroy();
		}
		flushkey();
	}
	void inputPassword(int bottomNum,int num,int x,int y,int sizeX,int sizeY,color_t bk,color_t word,int wordh,int wordw,char* pbuf,int lenbuf,int maxlen=50,bool if_bottom=false,bool flag=false,const char* ptext="ËÎÌå")
	{
		int temp=-1;
		int lenTemp=0;
		char str[20]={0},strTemp[20]={0};
		flushkey();
		if(bottom[bottomNum].flag==false)
			return;
		pinput[num].create(flag);
		pinput[num].move(x,y);
		pinput[num].size(sizeX+10,sizeY+10);
		pinput[num].setmaxlen(maxlen);
		pinput[num].visible(true);
		pinput[num].setbgcolor(bk);
		pinput[num].setcolor(word);
		pinput[num].setfont(wordh,wordw,ptext);
		pinput[num].setfocus();
		while(1)
		{
			memset(str,0,20);
			mouse=this->getMouse();
			if(mouse.is_left()&&mouse.x>bottom[bottomNum].xl&&mouse.x<bottom[bottomNum].xh\
			&&mouse.y>bottom[bottomNum].yl&&mouse.y<bottom[bottomNum].yh)
				break;
			pinput[num].gettext(lenbuf,strTemp);
			if(strTemp[0]!='*')
				strcpy(pbuf,strTemp);
			lenTemp=strlen(strTemp);
			for(int i=0;i<lenTemp;i++)
				strcat(str,"*");
			pinput[num].settext(str);
			this->update();
		}
		pinput[num].destroy();
	}
	void connectInputBottom(int bottomNum,int intputNum,int lenbuf,char* pbuf)
	{
		int temp=-1;
		while(1)
		{
			temp=this->waitMsgBottom();
			if(temp==bottomNum)
				break;
		}
		pinput[intputNum].gettext(lenbuf,pbuf);
		pinput[intputNum].destroy();
	}
	bool putBackPic(const char* pname,int w,int l)
	{
		getimage(img,pname);
		putimage(0,0,Width,Height,img,0,0,w,l);
		return true;
	}
	bool putBackPicRes(const char* ptype,const char* pname,int w,int l)
	{
		getimage(img,ptype,pname);
		putimage(0,0,Width,Height,img,0,0,w,l);
		return true;
	}//IMAGE_PNG_BG	PNG		"²¶»ñ.PNG"
	bool putBackPicNoZoom(const char* pname,int xl,int yl,int xh,int yh)
	{
		getimage(img,pname);
		putimage(0,0,Width,Height,img,xl,yl,xh,yh);
		return true;
	}
	inline void updateBkNoZoom(int xl,int yl,int xh,int yh)
	{
		putimage(0,0,Width,Height,img,xl,yl,xh,yh);
	}
	inline void updateBkPic(int w,int l)
	{
		putimage(0,0,Width,Height,img,0,0,w,l);
	}
	bool stickPic(int num,const char* pname,int w,int l,int x,int y,int xw,int yl)
	{
		pictu[num]=newimage(w,l);
		getimage(pictu[num],pname);
		putimage(x,y,xw,yl,pictu[num],0,0,w,l);
		return true;
	}
	bool stickPicRes(int num,const char* ptype,const char* pname,int w,int l,int x,int y,int xw,int yl)
	{
		pictu[num]=newimage(w,l);
		getimage(pictu[num],ptype,pname);
		putimage(x,y,xw,yl,pictu[num],0,0,w,l);
		return true;
	}
	inline void setTitle(const char* p)
	{
		setcaption(p);
	} 
	void setWord(int width,int height,const char* pmodel,color_t colorbk,color_t colorwd,bool flag=true)
	{
		setfont(width,height,pmodel);
		setfontbkcolor(colorbk);
		setcolor(colorwd);
		if(flag)
			setbkmode(TRANSPARENT);
	}
	inline color_t createColor(unsigned char red,unsigned char green,unsigned char blue)
	{
		return EGERGB(red,green,blue);
	}
	bool drawBottom(color_t color,int rex,int rey,int width,int height,int num,const char* pic=NULL,int pw=0,int ph=0)
	{
		if(num>=10||width>Width||height>Height)
			return false;
		setfillcolor(color);
		bar(rex,rey,width,height);
		bottom[num].xl=rex;
		bottom[num].xh=width;
		bottom[num].yl=rey;
		bottom[num].yh=height;
		bottom[num].flag=true;
		if(pic!=NULL&&pw>0&&ph>0)
			this->stickPic(9,pic,pw,ph,rex,rey,width,height);
		return true;
	}
	void cleanBottom(int i=-1)
	{
		if(i==-1)
		{
			for(int i=0;i<10;i++)
			{
				bottom[i].xl=0;
				bottom[i].xh=0;
				bottom[i].yl=0;
				bottom[i].yh=0;
				bottom[i].flag=false;
			}
		}
		else if(i>=0&&i<10)
		{
			bottom[i].xl=0;
			bottom[i].xh=0;
			bottom[i].yl=0;
			bottom[i].yh=0;
			bottom[i].flag=false;
		}
	}
	bool bottomName(const char* pname,int num,int x,int y)
	{
		if(bottom[num].flag==false)
			return false;
		if(x<bottom[num].xl||x>bottom[num].xh||\
		y<bottom[num].yl||y>bottom[num].yh)
			return false;
		xyprintf(x,y,"%s",pname);
		return true;
	}
	int waitMsgBottom()
	{
		while(1)
		{
			flushmouse();
			mouse=getmouse();
			if(mouse.is_left())
			{
				for(int i=0;i<10;i++)
				{
					if(bottom[i].flag==false)
						continue;
					if(mouse.x>bottom[i].xl&&mouse.x<bottom[i].xh\
					&&mouse.y>bottom[i].yl&&mouse.y<bottom[i].yh)
						return i;
				}
			}
		}
	}
	bool input(const char* ptitle,const char* ptext,char* buf,int len)
	{
		flushkey();
		return inputbox_getline(ptitle,ptext,buf,len);
	}
	inline void printfin(int x,int y,const char* ps)
	{
		xyprintf(x,y,"%s\n",ps);
	}
	inline mouse_msg getMouse()
	{
		flushmouse();
		return getmouse();
	}
	inline void delayTime(long t)
	{
		delay(t);
	}
	bool kbget(int* pget)
	{
		if(kbhit())
		{
			*pget=getch();
			return true;
		}
		else 
			return false;
	}
	inline void setBkColor(color_t color,PIMAGE pImg=NULL)
	{
		setbkcolor(color,pImg);
	}
	inline void borderFill(int x,int y,int color)
	{
		floodfill(x,y,color);
	}
	inline void areaColor(int x,int y,int color)
	{
		floodfillsurface(x,y,color);
	}
	void cleanDevice(bool flag=true)
	{
		if(flag)
			cleardevice(NULL);
		else
		{
			cleardevice(img);
		}
	}
	void setLine(color_t color,float thick)
	{
		setcolor(color);
		setlinewidth(thick);
	}
	inline void drawRect(int xl,int yl,int xh,int yh,color_t fill)
	{
		setfillcolor(fill);
		bar(xl,yl,xh,yh);
	}
	inline void drawLine(int x1,int y1,int x2,int y2)
	{
		line(x1,y1,x2,y2);
	}
	void drawEllipse(int x,int y,int xr,int yr,bool if_fill=false,color_t fill=0)
	{
		if(if_fill)
		{
	 		setfillcolor(fill);
			fillellipse(x,y,xr,yr);
		}
		else
		{
			ellipse(x,y,0,360,xr,yr);
		}
	}
	inline void resizeWindow(int x,int y)
	{
		resizewindow(x,y);
		this->Width=x;
		this->Height=y;
	}
	inline void update()
	{
		delay_ms(0);
	}
};
void test01()
{
	char mi[20]={0},hao[20]={0};
	class initGraphics pic(426,326,3);
	
	pic.setBkColor(WHITE); 
	pic.setTitle("QQµÇÂ¼");
	pic.putBackPicRes("PNG","IMAGE_PNG_BG",426,326);
	pic.drawBottom(pic.createColor(57,198,254),90,270,340,310,0);
	pic.setWord(20,10,"ËÎÌå",pic.createColor(57,198,254),WHITE);
	pic.bottomName("µÇÂ¼",0,195,280);
	pic.drawEllipse(210,110,50,50,true,BLUE);
	pic.printfin(110,110,"¡ð");
//	if(false==pic.musicPlay("MUSIC_MP3_BM"))
//		pic.printfin(0,0,"hahaha");
	pic.createInputBottom(0,1,110,203,195,18,WHITE,BLACK,18,9,mi,20,30);
	pic.createInputBottom(0,0,110,165,195,18,WHITE,BLACK,18,9,hao,20,30);
	pic.connectInputBottom(0,0,20,mi);
	pic.connectInputBottom(0,0,20,hao);
	
}
int main(int argc, char** argv) 
{
	int x=0,y=0;char sec[20]={0};
	test01();
	getch();
	return 0;
}
