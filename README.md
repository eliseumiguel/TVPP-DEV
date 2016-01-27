# TVPP-DEV
# TVPP-DEV

Deciões de implementação para a mesclagem

OBS -> Observar com clareza o que é necessário para o canal e o que é necessário para o servidor auxiliar.

1) Servidores Auxiliares em um mesmo canal não são parceiros.
   Motivo. Na rede principal, eles já não são parceiros, visto que eliminam as listas OUT ao iniciar tarefa de servidor. Eles, também, não conseguem trocar dados na rede paralela visto que os IDs de Subcannal de cada um deles é de rede principal. Finalmente, eles não tornam-se dependentes entre si e trazem outras fontes de vídeo ao subcanal
   
2) Os Servidores_Auxiliares configuram o ID-Sub-Channel em PeerData com o ID da rede principal. Isso é necessário para ele manter as parcerias na rede principal e, além disso, ele é sim pertencente à rede Principal. Para controle de qual canal ele server, a lista de servidores auxiliares ativos map<string, SubChannelServerAuxData> channel_Sub_List no channel gerencia os Servidores Auxiliares, onde o IdSub do canal que ele serve é configurado.

3) Observar que os pares da rede paralela mesclando e em estado normal não estão aparentando ter parceiros out... Isso pode ser um erro

4) inserção da classe peerManagerServerAux implica
   Mudanças na classe peerManager -> conferir o que tem comentário com //# (provavelmente será migrado para o peerManagerServerAux
   Mudanças no Client (muita atenção com isso)
      a) comentário no código que configura o estado do peer -> passará para o peerManagerServerAux
      b) função ping() onde é enviado ping para os pares na rede Master enquanto servidor
      
      erro é 
      client/client.cpp:984:59: error: ‘class PeerManager’ has no member named ‘GetPeerActiveOut_Master’
      client/client.cpp:1003:105: error: ‘class PeerManager’ has no member named ‘GetPeerActiveOut_Master’
      client/client.cpp:1004:60: error: ‘class PeerManager’ has no member named ‘GetPeerActiveOut_Master’
      client/client.cpp:1004:113: error: ‘class PeerManager’ has no member named ‘GetPeerActiveOut_Master’

