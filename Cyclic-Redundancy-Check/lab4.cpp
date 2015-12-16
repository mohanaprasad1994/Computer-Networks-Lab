#include<iostream>
using namespace std;
#include<sys/time.h>
#include<stdlib.h>
#include<fstream>
int N=50;

long long int message=0,message_to_send=0;
int generating_function=263;
int max_bits=32;

int find_rem(long long int numerator,int denominator){
    int i;
    int div=numerator>>32;
    for(i=31;i>=0;i--){
        div=((div<<1)|(((numerator>>i)&1)));
        if(((div>>8))==1)
            div=div^denominator;
        //cout<<div<< " ";
    }
    return div;
    }
        
    /*for(i=39;i>=8;i--)
        if(((numerator>>i)&1)==1){
            numerator=numerator^(denominator<<(i-8));
            cout<<numerator<<endl;
            return find_rem(numerator,denominator); 
            }
           
    cout<<numerator<<endl;        
    return numerator;*/
    //cout<<div;
    

int find_rem1(long long int numerator, int denominator,int present_num,int index){

    
    int i; 
     
    cout<<"before shifting :";   
    for(i=max_bits-1;i>=0;i--)
        cout<<((present_num>>i)&1);
    cout<<endl;    
    cout<<present_num<<endl;
    if (((present_num>>8)&1)==1)
        present_num=(present_num^denominator);
        
    
    
    
    present_num=present_num<<1;
    int temp=((numerator>>index)&1);
    
    present_num=present_num|temp;
    
    cout<<"After shiftinf  :";
    for(i=max_bits-1;i>=0;i--)
        cout<<((present_num>>i)&1);
    cout<<" "<<temp<<endl;
    cout<<present_num<<endl;    
    
    
    index--;
    if (index==-1){
        if (((present_num>>8)&1)==1)
            present_num=(present_num^denominator);
        return present_num;
        }
    
    
    return find_rem1(numerator, denominator,present_num,index);
} 

int main(int argc ,char * argv[]){
    ifstream fin(argv[1]);
    ofstream fout(argv[2]);
    
    srand(time(NULL));
    int t=0;
    while (t++<N){
    char a[50];
    fin>>a;
    int i;
    for(i=0;i<max_bits;i++){
        if (a[i]=='1')
            message=(message|1);
        message=message<<1;
    }
    message=message>>1;
    fout<<"Input: ";
    for(i=(max_bits-1);i>=0;i--)
        fout<<((message>>i)&1);
    fout<<endl;
    
    
    int temp=(message>>(max_bits-9));
    message=message<<8;
    
    //for(i=max_bits-1;i>=0;i--)
    //    cout<<((temp>>i)&1);
    //cout<<endl;
    
    //long long int reminder=find_rem(message,generating_function,temp,max_bits+8-9-1);
    long long int reminder=find_rem(message,generating_function);
    message_to_send=message^reminder;
    /*cout<<"reminder ";
    for(i=max_bits-1;i>=0;i--)
        cout<<((reminder>>i)&1);
    cout<<endl;
    cout<<reminder<<endl;*/
    fout<<"CRC: ";
    for(i=max_bits+8-1;i>=0;i--)
        fout<<((message_to_send>>i)&1);
     fout<<endl;
    fout<<"Original String: ";
    for(i=(max_bits-1);i>=0;i--)
        fout<<((message>>i)&1);
    fout<<endl;
    fout<<"Original String with CRC: ";
    for(i=max_bits+8-1;i>=0;i--)
        fout<<((message_to_send>>i)&1);
     fout<<endl;
    //cout<<endl<<reminder<<endl;
    int j;
    //cout<<(find_rem(message_to_send,generating_function))<<endl;
    
    //cout<<"1 bit errors: "<<endl;
    //  one bit errors
    for(i=0;i<10;i++){
        int index=rand()%40;
        message=(1<<i)^message_to_send;
        fout<<"Corrupted String: ";
        for(j=max_bits+8-1;j>=0;j--)
            fout<<((message>>j)&1);
        fout<<endl;
        fout<<"Number of Errors Introduced: 1"<<endl;
        if(find_rem(message,generating_function)==0)
            fout<<"CRC Check: Passed"<<endl;
        else
            fout<<"CRC Check: Failed"<<endl;
    }
    
    //cout<<"2 bit errors: "<<endl;
    // 2 bit errors
    for(i=0;i<10;i++){
        int index=rand()%40;
        message=(1<<i)^message_to_send;
        index=rand()%40;
        message=(1<<i)^message_to_send;
        fout<<"Corrupted String: ";
        for(j=max_bits+8-1;j>=0;j--)
            fout<<((message>>j)&1);
        fout<<endl;
        fout<<"Number of Errors Introduced: 2"<<endl;
        if(find_rem(message,generating_function)==0)
            fout<<"CRC Check: Passed"<<endl;
        else
            fout<<"CRC Check: Failed"<<endl;
    }
    
    //cout<<"2 bit errors: "<<endl;
    // odd number of bit errors
    for(i=0;i<10;i++){
        
        int index=rand()%35;
        int num=rand()%15;
        num=num*2+1;
        int num2=num;
        while(num>=0){
            num--;
            message=(1<<index)^message_to_send;
            index=(index+1)%40;
        }
        fout<<"Corrupted String: ";
        for(j=max_bits+8-1;j>=0;j--)
            fout<<((message>>j)&1);
        fout<<endl;
        fout<<"Number of Errors Introduced: "<<num2<<endl;
        if(find_rem(message,generating_function)==0)
            fout<<"CRC Check: Passed"<<endl;
        else
            fout<<"CRC Check: Failed"<<endl;
    }
    }      
        
    return 0;
}
    
    
