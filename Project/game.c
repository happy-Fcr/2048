/*========================= game.c =========================*/

#include "game.h"
#include "lcd_driver.h"

#include <stdio.h>
#include <stdlib.h>

int board[4][4];

static void add_random(void)
{
    int x,y;

    while(1)
    {
        x = rand()%4;
        y = rand()%4;

        if(board[x][y]==0)
        {
            board[x][y]=2;
            break;
        }
    }
}

void game_init(void)
{
    int i,j;

    for(i=0;i<4;i++)
    {
        for(j=0;j<4;j++)
        {
            board[i][j]=0;
        }
    }

    add_random();
    add_random();
}

void move_left(void)
{
    int i,j,k;

    for(i=0;i<4;i++)
    {
        for(j=0;j<4;j++)
        {
            if(board[i][j]==0)
            {
                for(k=j+1;k<4;k++)
                {
                    if(board[i][k]!=0)
                    {
                        board[i][j]=board[i][k];
                        board[i][k]=0;
                        break;
                    }
                }
            }
        }

        for(j=0;j<3;j++)
        {
            if(board[i][j]==board[i][j+1] && board[i][j]!=0)
            {
                board[i][j]*=2;
                board[i][j+1]=0;
            }
        }

        for(j=0;j<4;j++)
        {
            if(board[i][j]==0)
            {
                for(k=j+1;k<4;k++)
                {
                    if(board[i][k]!=0)
                    {
                        board[i][j]=board[i][k];
                        board[i][k]=0;
                        break;
                    }
                }
            }
        }
    }

    add_random();
}

void move_right(void)
{
    int i,j,k;

    for(i=0;i<4;i++)
    {
        for(j=3;j>=0;j--)
        {
            if(board[i][j]==0)
            {
                for(k=j-1;k>=0;k--)
                {
                    if(board[i][k]!=0)
                    {
                        board[i][j]=board[i][k];
                        board[i][k]=0;
                        break;
                    }
                }
            }
        }

        for(j=3;j>0;j--)
        {
            if(board[i][j]==board[i][j-1] && board[i][j]!=0)
            {
                board[i][j]*=2;
                board[i][j-1]=0;
            }
        }

        for(j=3;j>=0;j--)
        {
            if(board[i][j]==0)
            {
                for(k=j-1;k>=0;k--)
                {
                    if(board[i][k]!=0)
                    {
                        board[i][j]=board[i][k];
                        board[i][k]=0;
                        break;
                    }
                }
            }
        }
    }

    add_random();
}

void move_up(void)
{
    int i,j,k;

    for(j=0;j<4;j++)
    {
        for(i=0;i<4;i++)
        {
            if(board[i][j]==0)
            {
                for(k=i+1;k<4;k++)
                {
                    if(board[k][j]!=0)
                    {
                        board[i][j]=board[k][j];
                        board[k][j]=0;
                        break;
                    }
                }
            }
        }

        for(i=0;i<3;i++)
        {
            if(board[i][j]==board[i+1][j] && board[i][j]!=0)
            {
                board[i][j]*=2;
                board[i+1][j]=0;
            }
        }

        for(i=0;i<4;i++)
        {
            if(board[i][j]==0)
            {
                for(k=i+1;k<4;k++)
                {
                    if(board[k][j]!=0)
                    {
                        board[i][j]=board[k][j];
                        board[k][j]=0;
                        break;
                    }
                }
            }
        }
    }

    add_random();
}

void move_down(void)
{
    int i,j,k;

    for(j=0;j<4;j++)
    {
        for(i=3;i>=0;i--)
        {
            if(board[i][j]==0)
            {
                for(k=i-1;k>=0;k--)
                {
                    if(board[k][j]!=0)
                    {
                        board[i][j]=board[k][j];
                        board[k][j]=0;
                        break;
                    }
                }
            }
        }

        for(i=3;i>0;i--)
        {
            if(board[i][j]==board[i-1][j] && board[i][j]!=0)
            {
                board[i][j]*=2;
                board[i-1][j]=0;
            }
        }

        for(i=3;i>=0;i--)
        {
            if(board[i][j]==0)
            {
                for(k=i-1;k>=0;k--)
                {
                    if(board[k][j]!=0)
                    {
                        board[i][j]=board[k][j];
                        board[k][j]=0;
                        break;
                    }
                }
            }
        }
    }

    add_random();
}

void game_display(void)
{
    int i, j;
    char buf[10];

    LCD_Clear(BLACK);

    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 4; j++)
        {
            if(board[i][j] != 0)
                sprintf(buf, "%d", board[i][j]);
            else
                sprintf(buf, ".");

            LCD_ShowString(
                20 + j * 50,
                40 + i * 40,
                buf,
                WHITE,
                BLACK
            );
        }
    }
}

