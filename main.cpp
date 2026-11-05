#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include "common/Intruder.h"
using namespace std;
Intruder intruders[100];
int count = 0;
int linearSearch(Intruder arr[], int size, int key)
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i].id == key)
        {
            return i;
        }
    }
    return -1;
}
int binarySearch(int arr[], int size, int key)
{
    int low = 0;
    int high = size - 1;
    while (low <= high)
    {
        int mid = (low + high) / 2;
        if (arr[mid] == key)
        {
            return mid;
        }
        else if (key < arr[mid])
        {
            high = mid - 1;
        }
        else
        {
            low = mid + 1;
        }
    }
    return -1;
}
void bubbleSort(int arr[], int size)
{
   for (int i = 0; i < size - 1; i++)
    {
        for (int j = 0; j < size - i - 1; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}
void bubbleSortIntruders(Intruder arr[], int size)
{
   for(int i = 0; i < size-1; i++)
   {
      for(int j = 0; j < size-i-1; j++)
      {
         if(arr[j].id > arr[j+1].id)
         {
            Intruder temp = arr[j];
            arr[j] = arr[j+1];
            arr[j+1] = temp; 
         }
      }
   }
}
void selectionSortIntruders(Intruder arr[], int size)
{
    for(int i = 0; i < size - 1; i++)
    {
        int minIndex = i;

        for(int j = i + 1; j < size; j++)
        {
            if(arr[j].id < arr[minIndex].id)
            {
                minIndex = j;
            }
        }

        if(minIndex != i)
        {
            Intruder temp = arr[i];
            arr[i] = arr[minIndex];
            arr[minIndex] = temp;
        }
    }
}
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
void sortintruderrecords()
{
    if (count == 0)
    {
        cout << "\nNO RECORDS AVAILABLE!" << endl;
        return;
    }

    Intruder sortedIntruders[100];

    for (int i = 0; i < count; i++)
    {
        sortedIntruders[i] = intruders[i];
    }

    bubbleSortIntruders(sortedIntruders, count);

    cout << "\n======= SORTED INTRUDER RECORDS =======" << endl;

    for (int i = 0; i < count; i++)
    {
        cout << "\nINTRUDER ID       : " << sortedIntruders[i].id << endl;
        cout << "INTRUDER NAME     : " << sortedIntruders[i].name << endl;
        cout << "INTRUDER LOCATION : " << sortedIntruders[i].location << endl;
        cout << "INTRUDER THREAT LEVEL : " << sortedIntruders[i].threatlevel << endl;
        cout << "THREAT CATEGORY   : " << threatcategory(sortedIntruders[i].threatlevel) << endl;
        cout << "-----------------------------------" << endl;
    }
}
void selectionsortintruderrecords()
{
    if(count == 0)
    {
        cout << "No intruder records available!" << endl;
        return;
    }

    Intruder sortedIntruders[100];

    for(int i = 0; i < count; i++)
    {
        sortedIntruders[i] = intruders[i];
    }

    selectionSortIntruders(sortedIntruders, count);

    cout << "\n======= SELECTION SORTED INTRUDER RECORDS =======" << endl;

    for(int i = 0; i < count; i++)
    {
        cout << "\nINTRUDER ID       : " << sortedIntruders[i].id << endl;
        cout << "INTRUDER NAME     : " << sortedIntruders[i].name << endl;
        cout << "INTRUDER LOCATION : " << sortedIntruders[i].location << endl;
        cout << "INTRUDER THREAT LEVEL : " << sortedIntruders[i].threatlevel << endl;
        cout << "THREAT CATEGORY   : " << threatcategory(sortedIntruders[i].threatlevel) << endl;
        cout << "-----------------------------------" << endl;
    }
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
   cout<<"\nENTER INTRUDER ID TO SEARCH: ";
   cin>> searchid;
   int result = linearSearch(intruders, count, searchid);
   if(result != -1)
   {
      cout << "\nINTRUDER FOUND!" << endl;
        cout << "----------------------------" << endl;

        cout << "INTRUDER ID       : " << intruders[result].id << endl;
        cout << "INTRUDER NAME     : " << intruders[result].name << endl;
        cout << "INTRUDER LOCATION : " << intruders[result].location << endl;
        cout << "INTRUDER THREAT LEVEL : " << intruders[result].threatlevel << endl;
        cout << "THREAT CATEGORY   : " << threatcategory(intruders[result].threatlevel) <<endl;
        cout << "---------------------------------" << endl;
    }
    else
    {
        cout << "\nINTRUDER NOT FOUND!" << endl;
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
    cout << "8. Sort Intruder Records by ID" << endl;
    cout << "9. Selection Sort Intruder Records by ID" << endl;
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
            cout << " Stay Safe! Goodbye!" << endl;
            cout << "=========================================" << endl;
            break;
       case 7:
           systemstatistics();
           break;
         case 8:
            sortintruderrecords();
            break;
         case 9:
             selectionsortintruderrecords();
             break;
        default:
                cout << "Invalid Choice!" << endl;
      }
    } while(choice != 6);

    return 0;
}

