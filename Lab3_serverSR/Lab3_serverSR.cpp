// Lab3_serverSR.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <string>   
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <filesystem>
#include <fstream>

#pragma comment(lib,"ws2_32")

using namespace std; 
// pour recupérer l'information obtenus grace a la commande executé, nous avons utiliser une fonction sur internet qui crée un fichier temporaire 
// dont la console (qui execute la commande) écris a l'intérieur le resultat (ce qui est retourner)
// nous avons trouver sur ce site:
//https://rosettacode.org/wiki/Get_system_command_output

wstring RépertoireCourant()//code trouvé à :https://stackoverflow.com/questions/875249/how-to-get-current-directory
{
    TCHAR buffer[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buffer, MAX_PATH);
    wstring::size_type pos = wstring(buffer).find_last_of(L"\\/");
    return wstring(buffer).substr(0, pos);
}

string execute(const string& command,int &value) {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    value=system((command + " > temp.txt").c_str());
    ifstream ifs("temp.txt");
    string ret{ istreambuf_iterator<char>(ifs), istreambuf_iterator<char>() };
    ifs.close(); // must close the inout stream so the file can be cleaned up
    if (remove("temp.txt") != 0) {
        perror("Error deleting temporary file");
    }
    return ret;
}

int main()
{

    // liste d'utilisateur autoriser le client envoie son identifiant et son mot de passe

  
    
    // sert à renouveler le serveur entre les différentes connection de client(après une deconnection)
    while (true)
    {

    

    vector<string>  listeUser = { "allo,123","hunter123,manger"};

    WSADATA wsaData;
    SOCKET serverSocket = INVALID_SOCKET,clientSocket = INVALID_SOCKET;
    struct addrinfo*pResult = NULL;
    struct addrinfo hints;

    char* sendBuffer;

    char recvBuffer[512];

    // Débute le processus WS2_32 sur la version 2,2
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        cout << "erreur lors de l'initiation de WinsockDLL\n";
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, "27015", &hints, &pResult) != 0)
    {
        cout << "erreur lors de la recuperation d'info du serveur\n";
        //termine le process WinsockDLL
        WSACleanup();
        return 1;
    }


    serverSocket = socket(pResult->ai_family, pResult->ai_socktype, pResult->ai_protocol);

    if (serverSocket == INVALID_SOCKET)
    {
        cout << "erreur lors de la connection du client\n";
        freeaddrinfo(pResult);
        WSACleanup();
        return 1;
    }

    int limitTailleFichier = 1048576;
    setsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, to_string(limitTailleFichier).c_str(), 1048576);

    if (bind(serverSocket,pResult->ai_addr,pResult->ai_addrlen) == SOCKET_ERROR)
    {
        cout << "erreur socket server\n";
        freeaddrinfo(pResult);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }


    freeaddrinfo(pResult);

    // connection établis (ou non) le socket du server n'est plus nécessaire
   // closesocket(serverSocket);
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        cout << "erreur listen socket server\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET)
    {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    closesocket(serverSocket);


    bool authentifier = false;

    do
    {
       // l'utilisateur s'identifie ici, il ne peut pas bypasser le if tant qu'il n'est pas identifier

        if (authentifier == false && recv(clientSocket, recvBuffer, 512, 0) > 0)
        {
         
            string tempo(recvBuffer, strlen(recvBuffer));
            tempo = tempo.substr(0,tempo.find_first_of("&&"));
           
            for (int i = 0; i < listeUser.size(); i++)
            {
             
                if (listeUser.at(i) == tempo)
                {
                    cout << "utilisateur authentifier" << "\n";
                    authentifier = true;
                   const char* reussi = "Authentification reussi";

                   // bizarrement lorsque j'envoie ici, il retourne un SOCKET_ERROR mais envoie quand même le message
                   // au client
                   send(clientSocket,reussi,strlen(reussi),0);
                   memset(recvBuffer, 0, sizeof recvBuffer);
                  
                }
            }

        }

        if (authentifier == true) {
            recv(clientSocket, recvBuffer, 3, 0);
            
            vector<string> listeFichierPath;
            vector<string> listeFichierFormatter;
            string nomCommande;
            // cas 1

            string tempoChoix(recvBuffer, strlen(recvBuffer));

            
            // ici on recupere la commande qui a été envoyer au cas ou ce qui est recu est l'envoie d'une commande
          

            tempoChoix = tempoChoix.substr(0, tempoChoix.find_first_of("&&"));


            stringstream directoryPath;
            //string tempp = "\\Fichier";
           // wstring Fichier(tempp.begin(), tempp.end());
            wstring temp= RépertoireCourant();
            string tempp(temp.begin(), temp.end());
            directoryPath << tempp << "\\Fichier";

            cout << directoryPath.str() << "\n";
      

            //wstring stringPath = temp + Fichier;

            //string directory ={filesystem::current_path()};


            // ici ajouter un path dynamique en envoyant une commande de la part du client
            //
            //
            //
            //directoryPath << filesystem::current_path().u8string() << "\\Fichier";

            // cout << directoryPath.str() << "\n";

             // Pour récupérer la liste des fichiers, j'ai utiliser filesystem (version c++17)
             // solution prise sur stackoverflow proposer par Shreevardhan : https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c/37494654#37494654
             // en utilisant filesystem depuis c++17
            int i = 0;
            int j = 0;
            // utilisation de deux boucle for pour connaître le nombre de packet à envoyé, comme sa le client s'attend à recevoir 8 nom de dossier s'il y en a 8
            //vraiment pas optimiser, mais c'était la premiere solution qui m'est venu pour
            //connaître le nombre de packet qui va être envoyer au client
            //comme sa si il y a une interruption le client pourra communiquer lui qu'il a raté
            // 
            for (const auto& entry : filesystem::directory_iterator(directoryPath.str()))
                i++;


            cout << "boucleFileSystem" << "\n";

            string tempo1 = to_string(i) + "&&";

            send(clientSocket, tempo1.c_str(), 512, 0);

            cout << tempo1 << "\n";


            if (tempoChoix == "1")
            {
               // directoryPath << filesystem::current_path().u8string() << "\\Fichier";
                
                //envoie le nombre de packet
               

                //envoie les packets (path des fichiers disponible pour que le client choisit)

                for (const auto& entry : filesystem::directory_iterator(directoryPath.str()))
                {
                    j++;
                    stringstream sstreamTempo;
                    sstreamTempo << j << ") " << entry.path().u8string() << "&&";
                    string tempo = sstreamTempo.str();
                    //i = i + tempo.size();
                    listeFichierPath.push_back(entry.path().u8string());
                    listeFichierFormatter.push_back(tempo);
                }


                cout << "on a passer filesystem de tempochoix1" << "\n";
                
                //envoie la taille du fichier
             

                stringstream EnvoieListeFormatter;
                string sstreamTOstring;

                for (int j = 0; j < listeFichierFormatter.size(); j++)
                {
                    EnvoieListeFormatter << listeFichierFormatter.at(j);
                }
                sstreamTOstring = EnvoieListeFormatter.str();
                const char* envoieNomFichier = sstreamTOstring.c_str();

                int intTempo = sstreamTOstring.size();

                cout << to_string(sstreamTOstring.size()).c_str();

                try
                {
                    send(clientSocket, to_string(sstreamTOstring.size()).c_str(), 512, 0);
                    int test =WSAGetLastError();
                    cout << test;
                }

                catch (const std::exception& e) // reference to the base of a polymorphic object
                {
                    std::cout << e.what(); // information from length_error printed
                }


                //send(clientSocket, to_string(sstreamTOstring.size()).c_str(), 512, 0);
                memset(recvBuffer, 0, sizeof recvBuffer);

                cout << "taille envoyer "  << sstreamTOstring.size() << "\n";





                // envoie des fichiers

                cout << envoieNomFichier << "\n";


                send(clientSocket, envoieNomFichier,sstreamTOstring.size(), 0);

                cout << "fichier envoyer "  << "\n";

                //En attente d'une réponse du client pour le fichier qu'il veut recupérer
                memset(recvBuffer, 0, sizeof recvBuffer);
                recv(clientSocket, recvBuffer, 512, 0);
                string tempo(recvBuffer, strlen(recvBuffer));

                cout << "on plante ici? " << "\n";
               
                //maintenant tempo est l'index-1 à récupérer dans le vecteur
                // listeFichierPath pour transferer le fichier au client
                //listeFichierPath.at(stoi(tempo));
                
                tempo = listeFichierPath.at(stoi(tempo)-1);
               // tempo = tempo.substr(0, tempo.find_first_of("&&"));

                replace(tempo.begin(), tempo.end(), '\\', '/');
                //tempo = tempo.substr(tempo.rfind("Fichier"),tempo.size());

                //tempo = tempo.substr(tempo.find_last_of("Fichier")-6, tempo.size());
                
                ifstream file;

                file.open(tempo, ios::in | ios::binary | ios::ate);
                long size = file.tellg();

                cout << "taille: " << size << "\n";
                //envoie la taille du fichier au client
               

                send(clientSocket,to_string(size).c_str(), strlen(to_string(size).c_str()), 0);

                file.seekg(0, ios::beg);
                sendBuffer = new char[size];

                //lis le fichier et écrit dans le buffer
                file.seekg(0, ios::beg);
                sendBuffer = new char[size];
                file.read(sendBuffer, size);
                file.close();

                //envoie le fichier
                send(clientSocket, sendBuffer, size, 0);
               // memset(sendBuffer, 0, sizeof sendBuffer);

            }

            if (tempoChoix == "2")
            {
                // on recoit la taille de la commande qui sera envoyer
                int tailleCommande;
                recv(clientSocket, (char*)&tailleCommande, 512, 0);

                //string tailleCommande(recvBuffer, sizeof recvBuffer);

                memset(recvBuffer, 0, sizeof recvBuffer);


                // ici on recoit la commande de taille qui a été recu précédament
                recv(clientSocket, recvBuffer,tailleCommande, 0);
               

                //                string commandeRecu = "";

            
                string commandeRecu(recvBuffer, sizeof recvBuffer);

                memset(recvBuffer, 0, sizeof recvBuffer);

                nomCommande = commandeRecu;


                int valueSysteme;
                string messageErreurCommande;
                stringstream tempomessage;


            // lorsqu'on exécute system, il en retourne si:fhfdghgddsfsfsdfs
            //0 si c'est correct
            //1 si la commande est invalide, peut être manque argument
            //2 si la commande est valide, pas exécutée
                //
                string RésultatCommande;
               // nomCommande;
                
                cout << "Ici on bug";
                RésultatCommande=execute(nomCommande,valueSysteme);

                cout << "Enfaites, on bug pas";
              //  cout << RésultatCommande << "\n";


                if (valueSysteme == 0 )
                {

                    tempomessage << "La commande a ete execute en voici le resultat\n" << RésultatCommande;

                    messageErreurCommande = tempomessage.str();

                    // envoie de la taille du messsage de la commande
                    send(clientSocket, to_string(messageErreurCommande.size()).c_str(), 512, 0);


                    cout << messageErreurCommande.size() << "\n";

                    


                    // envoie du message de la commande

                    send(clientSocket, messageErreurCommande.c_str(), messageErreurCommande.size(), 0);

                }
                else if (valueSysteme == 1)
                {
                    messageErreurCommande = "Il y a eu une erreur, commande invalide ou argument manquant";
                 
                    // envoie de la taille du messsage de la commande
                    send(clientSocket, to_string(messageErreurCommande.size()).c_str(), 512, 0);


                    // envoie du message de la commande
                    send(clientSocket, messageErreurCommande.c_str(), messageErreurCommande.size(), 0);
                }
                else if (valueSysteme)
                {
                    messageErreurCommande = "Commande entree non valide";
                    send(clientSocket, to_string(messageErreurCommande.size()).c_str(), 512, 0);


                    // envoie du message de la commande
                    send(clientSocket, messageErreurCommande.c_str(), messageErreurCommande.size(), 0);
                }

                else
                {
                    messageErreurCommande = "Erreur inconnue, non pris en charge";

                    send(clientSocket, to_string(messageErreurCommande.size()).c_str(), 512, 0);


                    // envoie du message de la commande
                    send(clientSocket, messageErreurCommande.c_str(), messageErreurCommande.size(), 0);
                }



                // pour recupérer l'information obtenus grace a la commande executé, nous avons utiliser une fonction sur internet qui crée un fichier temporaire 
                // dont la console (qui execute la commande) écris a l'intérieur le resultat (ce qui est retourner)
                // nous avons trouver sur ce site:
                //https://rosettacode.org/wiki/Get_system_command_output

        }
        if (tempoChoix == "3")
        {
                authentifier == false;
                break;
        }


    }
        memset(recvBuffer, 0, sizeof recvBuffer);

    } while (true);

        int tempo;
        tempo = shutdown(clientSocket, SD_SEND);
        if (tempo == SOCKET_ERROR)
        {
            cout << "Erreur lors de la fermeture du socket client";
            closesocket(clientSocket);
            WSACleanup();
        }
        closesocket(clientSocket);
        WSACleanup();


    }

    
 
}
