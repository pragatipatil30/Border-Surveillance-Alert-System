#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
using namespace std;

struct Intruder
{
   int id;
   string name;
   string location;
   int threatlevel;
};
Intruder intruders[100];
int count = 0;
string threatcategory(int level);
void saverecords();
void loadrecords();
void systemstatistics();
void addintruder()
{
   if(count == 100)
   {
      cout<<"DATABASE IS FULL!" <<endl;
      return;
   }
    cout<<"\n------------ADD INTRUDER RECORD-------------" << endl;
    cout<<"ENTER INTRUDER ID: ";
    cin>>intruders[count].id;
    for(int i = 0; i < count; i++)
    {
      if(intruders[count].id == intruders[i].id)
      {
         cout<<"\nINTRUDER ID ALREADY EXISTS" <<endl;
         cout<<"PLEASE ENTER A UNIQUE ID" << endl;
         return;
      }
    }
    cout<<"ENTER INTRUDER NAME: ";
    cin>>intruders[count].name;
    cout<<"ENTER LOCATION: ";
    cin>>intruders[count].location;
    cout<<"ENTER THREAT LEVEL(1-5): ";
    cin>>intruders[count].threatlevel;
    while(intruders[count].threatlevel < 1 || intruders[count].threatlevel > 5)
    {
       cout << "INVALID THREAT LEVEL! ENTER BETWEEN 1 AND 5: ";
       cin >> intruders[count].threatlevel;
    }
    cout<<"\nRECORD ADDED SUCCESSFULLY!" << endl;
    cout<<"-------------------------------" << endl;
    cout<<"INTRUDER ID     :" << intruders[count].id << endl;
    cout<<"INTRUDER NAME     :" << intruders[count].name << endl;
    cout<<"INTRUDER LOCATION     :" << intruders[count].location << endl;
    cout<<"INTRUDER THREAT LEVEL     :" << intruders[count].threatlevel << endl;
    cout<<"---------------------------------"<< endl;
    count++;
    saverecords();
}
void displayrecords()
{
   cout<<"\n===================================" << endl;
   cout<<"        ALL INTRUDER RECORDS         "  << endl;
   cout<<"===================================" << endl;
   if(count == 0)
   {
      cout<<"NO RECORDS AVAILABLE!" <<endl;
   }
   else
   {
      for(int i = 0; i<count; i++)
      {
         cout<<"\nINTRUDER RECORD "<<i+1 << endl;
         cout<<"-------------------------------" << endl;
         cout<<"INTRUDER ID           : " << intruders[i].id << endl;
         cout<<"INTRUDER NAME         : " << intruders[i].name << endl;
         cout<<"INTRUDER LOCATION     : " << intruders[i].location << endl; 
         cout<<"INTRUDER THREAT LEVEL : " << intruders[i].threatlevel << endl;
         cout<<"THREAT CATEGORY       : " << threatcategory(intruders[i].threatlevel) << endl;
         cout<<"-------------------------------" << endl;

      }
   }
   cout << "=================================" << endl;
}
void searchintruder()
{
   int searchid;
   bool found = false;
   cout<<"\nENTER INTRUDER ID TO SEARCH: ";
   cin>> searchid;
   for(int i = 0; i < count; i++)
   {
   
      if(searchid == intruders[i].id)
      {
          cout<<"\nINTRUDER FOUND!" << endl;
          cout<<"----------------------------" << endl;
          cout<<"INTRUDER ID       :" <<intruders[i].id<<endl;
          cout<<"INTRUDER NAME       :" <<intruders[i].name<<endl;
          cout<<"INTRUDER LOCATION       :" <<intruders[i].location<<endl;
          cout<<"INTRUDER THREAT LEVEL :" << intruders[i].threatlevel << endl;
          cout<<"THREAT CATEGORY       :" << threatcategory(intruders[i].threatlevel) << endl;
          cout << "---------------------------------" << endl;
          found = true;
          break;
      }
   }
   if(found == false)
   {
      cout<<"\nINTRUDER NOT FOUND!" << endl;
   }
}
void updatethreatlevel()
{
   int updateid;
   bool found = false;
   cout<<"\nENTER INTRUDER ID TO UPDATE: ";
   cin>>updateid;
   for(int i = 0; i < count; i++)
   {
      if(updateid == intruders[i].id)
      {
         cout<< "ENTER NEW THREAT LEVEL (1-5): ";
         cin>>intruders[i].threatlevel;
         while(intruders[i].threatlevel < 1 || intruders[i].threatlevel > 5)
         {
            cout << "INVALID THREAT LEVEL! ENTER BETWEEN 1 AND 5: ";
            cin >> intruders[i].threatlevel;
         }
         cout << "\nTHREAT LEVEL UPDATED SUCCESSFULLY!" << endl;
         cout<<"----------------------------" << endl;
         cout<<"INTRUDER ID       : " << intruders[i].id << endl;
         cout<<"INTRUDER NAME       :" <<intruders[i].name<<endl;
         cout<<"INTRUDER LOCATION       :" <<intruders[i].location<<endl;
         cout<<"INTRUDER THREAT LEVEL :" << intruders[i].threatlevel << endl;
         cout<<"THREAT CATEGORY       :" << threatcategory(intruders[i].threatlevel) << endl;
         cout << "---------------------------------" << endl;
         saverecords();
         found = true;
         break;
      }     
   }
   if(found == false)
   {
      cout<<"\nINTRUDER NOT FOUND!" << endl;
   } 
}
void deleteintruder()
{
   int deleteid;
   bool found = false;
   cout<<"\nENTER INTRUDER ID TO DELETE: ";
   cin>>deleteid;
   for(int i = 0; i < count; i++)
   {
      if(deleteid == intruders[i].id)
      {
         char choice;

         cout << "ARE YOU SURE YOU WANT TO DELETE THIS RECORD? (Y/N): ";
         cin >> choice;

         if(choice == 'Y' || choice == 'y')
         {
            for(int j = i; j < count - 1; j++)
            {
               intruders[j] = intruders[j + 1];
            }

            count--;
            saverecords();

            cout << "\nINTRUDER RECORD DELETED SUCCESSFULLY!" << endl;
         }
         else
         {
            cout << "\nDELETE OPERATION CANCELLED!" << endl;
         }
         found = true;
         break;
      }
   }
   if(found == false)
    {
        cout<< "\nINTRUDER NOT FOUND!" << endl;
    }
}
string threatcategory(int level)
{
   if(level == 1 || level == 2)
   {
      return "LOW";
   }
   else if(level == 3)
   {
      return "MEDIUM";
   }
   else
   {
      return "HIGH";
   }
}
void saverecords()
{
   ofstream file("intruders.txt");
   for(int i = 0; i < count; i++)
   {
      file<< intruders[i].id <<" "
          << intruders[i].name <<" "
          << intruders[i].location << " "
          << intruders[i].threatlevel << endl;
   }
   file.close();
}
void loadrecords()
{
   ifstream file("intruders.txt");
   if(!file)
   {
      return;
   }
   count = 0;
   while(count < 100 &&
         file >> intruders[count].id
              >> intruders[count].name
              >> intruders[count].location
              >> intruders[count].threatlevel)
   {
      count++;
   }
   file.close();
}
void systemstatistics()
{
    int low = 0;
    int medium = 0;
    int high = 0;
    for(int i = 0; i < count; i++)
    {
        if(intruders[i].threatlevel == 1 || intruders[i].threatlevel == 2)
        {
            low++;
        }
        else if(intruders[i].threatlevel == 3)
        {
            medium++;
        }
        else
        {
            high++;
        }
    }
    cout << "\n========== SYSTEM STATISTICS ==========" << endl;
    cout << "TOTAL RECORDS : " << count << endl;
    cout << "LOW THREAT    : " << low << endl;
    cout << "MEDIUM THREAT : " << medium << endl;
    cout << "HIGH THREAT   : " << high << endl;
    cout << "=======================================" << endl;
}
void login()
{
   string username, password;
   int attempts = 3;
   while(attempts > 0)
   {
      cout<<"\n============= ADMIN LOGIN ==============" << endl;
      cout<<"ENTER USERNAME: ";
      cin>>username;
      cout<<"PASSWORD: ";
      cin>>password;
      if(username == "pragati" && password == "pragati123")
      {
        cout<<"\nLOGIN SUCCESSFUL!"<<endl;
        cout << "WELCOME, PRAGATI!" << endl;
        return;
      }
      else
      {   
         attempts--;
         cout << "\nINVALID USERNAME OR PASSWORD!" << endl;
         if(attempts > 0)
         {
            cout<< "ATTEMPTS LEFT: " << attempts << endl;
         }
      }
   }
   cout << "\n======================================" << endl;
   cout << "MAXIMUM LOGIN ATTEMPTS EXCEEDED!" << endl;
   cout << "PROGRAM TERMINATED." << endl;
   cout << "======================================" << endl;
   exit(0);
}
void menu()
{
    cout << "=========================================" << endl;
    cout << "     BORDER SURVEILLANCE ALERT SYSTEM    " << endl;
    cout << "=========================================" << endl;
    cout << "1. Add Intruder Record" << endl;
    cout << "2. Display All Records" << endl;
    cout << "3. Search Intruder" << endl;
    cout << "4. Update Threat Level" << endl;
    cout << "5. Delete Record" << endl;
    cout << "6. Exit SYSTEM" << endl;
    cout << "7. System Statistics" << endl;
    cout << "=========================================" << endl;
}
int main()
{
    loadrecords();
    login();
    int choice;
    do
    {
      menu();

      cout << "Enter your choice: ";
      cin >> choice;
      switch(choice)
      {
        case 1:
           addintruder();
           break;
        case 2:
           displayrecords();
           break;
        case 3:
           searchintruder();
           break;
        case 4:   
           updatethreatlevel();
           break;
        case 5:
           deleteintruder();
           break;
        case 6:
            cout << "\n=========================================" << endl;
            cout << "   THANK YOU FOR USING THE SYSTEM" << endl;
            cout << "=========================================" << endl;
            cout << " BORDER SURVEILLANCE ALERT SYSTEM" << endl;
            cout << " Developed By: Pragati Patil" << endl;
            cout << " Stay Safe! Goodbye!" << endl;
            cout << "=========================================" << endl;
            break;
       case 7:
           systemstatistics();
           break;
        default:
                cout << "Invalid Choice!" << endl;
      }
    } while(choice != 6);

    return 0;
}

