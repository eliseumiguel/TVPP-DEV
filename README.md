# TVPP-DEV
# TVPP-DEV

Deciões de implementação para a mesclagem


1) Servidores Auxiliares em um mesmo canal não são parceiros.
   Motivo. Na rede principal, eles já não são parceiros, visto que eliminam as listas OUT ao iniciar tarefa de servidor. Eles, também, não conseguem trocar dados na rede paralela visto que os IDs de Subcannal de cada um deles é de rede principal. Finalmente, eles não tornam-se dependentes entre si e trazem outras fontes de vídeo ao subcanal
   
2) Os Servidores_Auxiliares configuram o ID-Sub-Channel em PeerData com o ID da rede principal. Isso é necessário para ele manter as parcerias na rede principal e, além disso, ele é sim pertencente à rede Principal. Para controle de qual canal ele server, a lista de servidores auxiliares ativos map<string, SubChannelServerAuxData> channel_Sub_List no channel gerencia os Servidores Auxiliares, onde o IdSub do canal que ele serve é configurado.

