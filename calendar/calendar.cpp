#include "calendar.h"
//������ռ���
WORD ifFinalDay(WORD t_year,WORD t_month){
	WORD t_monthDays = 0;
	WORD t_weekDay = 0;
	t_monthDays = MonthDays( t_year,t_month);
	int totalFriday = 0;//ͳ��һ�����ٸ������壬���������������������
	for(WORD i = 1;i <= t_monthDays;i++){
		t_weekDay = WeekDay(t_year,t_month,i);
		if(t_weekDay ==  5){
			totalFriday++;
		}
		if(totalFriday >= 3){//����3����;��������岻�ǽ�����,��ȡ����3�����һ��������	
			if(isTradeDay(t_year,t_month,i)){
				return i;
			}
		}
	}
	return t_monthDays;//û���ҵ��������,�����������Ϊt_monthDays��,��������������ڶ����л���Լ
}
WORD A50FinalDay(WORD t_year,WORD t_month){
	WORD t_monthDays = 0;
	t_monthDays = MonthDays( t_year,t_month);
	int total  = 0;//������ʼ�Ľ�������ͳ��
	for(WORD i = t_monthDays;i >= 1;i--){
		if(isTradeDay(t_year,t_month,i)){
			total++;
		};
		if(total == 2){
			return i;//���һ��������
		}
	}
	return t_monthDays;//û���ҵ��������,�����������Ϊt_monthDays��,��������������ڶ����л���Լ
}
bool isTradeDay(WORD t_year,WORD t_month,WORD t_day){
	WORD t_weekday = 0;
	t_weekday = WeekDay(t_year,t_month,t_day);
	//��������	
	if(t_weekday == 6 || t_weekday == 0){
		return false;
	}
	if(t_year ==  2014){
		if(t_month == 1 && t_day == 31){
			return false;
		}
		else if(t_month == 2 && t_day >= 1 && t_day <= 6){
			return false;
		}
		else if(t_month == 4 && t_day >= 5 && t_day <= 7){
			return false;
		}
		else if(t_month == 5 && t_day >= 1 && t_day <= 3){
			return false;
		}
		else if(t_month == 5 && t_day == 31){
			return false;
		}
		else if(t_month == 6 && t_day >= 1 && t_day <= 2){
			return false;
		}
		else if(t_month == 9 && t_day >= 6 && t_day <= 8){
			return false;
		}
		else if(t_month == 10 && t_day >= 1 && t_day <= 7)
		{
			return false;
		}
		else{
			//������
			return true;
		}
	}
	else{
		//��һ���������������¸��½�����
		return true;
	}
}
WORD MonthDays(WORD year, WORD month)//�����������ź��·ݣ����ظ��µ�����    
{
	switch(month)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:  return 31;
	case 4:
	case 6:
	case 9:
	case 11:  return 30;
	case 2:   if(year%4==0 && year%100!=0 || year%400==0)
				  return 29; 
			  else
				  return 28;
	default: return 0;  
	}
}
WORD WeekDay(WORD year,WORD month,WORD day)
{
	long sum;
	WORD i;
	//��������
	sum=((year-1)*365+(year-1)/4+(year-1)/400-(year-1)/100);
	for(i=1;i<month;i++)
	{
		sum+=MonthDays(year,i);
	}
	sum+=day;
	return (WORD)(sum%7);
}