#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "Chat.h"

class gamble_npc : public CreatureScript
{
public:
    gamble_npc() : CreatureScript("gamble_npc") {}

    uint32 CostoCobre = 400000; // 40 oro

    uint32 ObtenerEmblemaAleatorio()
    {
        switch (urand(1, 4))
        {
        case 1:
            return 40752; // Heroism
        case 2:
            return 40753; // Valor
        case 3:
            return 45624; // Conquest
        case 4:
            return 47241; // Triumph
        }
        return 40752;
    }

    bool OnGossipHello(Player *player, Creature *creature) override
    {
        player->PlayerTalkClass->ClearMenus();

        AddGossipItemFor(player, 10, "|TInterface\\icons\\inv_misc_coin_02:40:40:-18|t Jugar al Casino (Costo: 40 de Oro)", GOSSIP_SENDER_MAIN, 1);
        AddGossipItemFor(player, 10, "|TInterface\\icons\\inv_misc_dice_01:40:40:-18|t ¿Cómo funciona?", GOSSIP_SENDER_MAIN, 2);

        SendGossipMenuFor(player, 1, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        switch (action)
        {
        case 1:
            Apostar(player, creature);
            break;

        case 2:
            ChatHandler(player->GetSession()).SendNotification("|cffFFD700REGLAS DEL CASINO\n"
                                                               "|cffFFFFFFPaga 40 de oro para tirar los dados.\n"
                                                               "Un número alto gana premio.\n"
                                                               "El 100 gana el JACKPOT.");
            CloseGossipMenuFor(player);
            break;
        }

        return true;
    }

    void Apostar(Player *player, Creature *creature)
    {
        if (player->GetMoney() < CostoCobre)
        {
            creature->Whisper("Necesitas 40 de oro para jugar.", LANG_UNIVERSAL, player);
            CloseGossipMenuFor(player);
            return;
        }

        player->ModifyMoney(-int32(CostoCobre));

        uint32 roll = urand(1, 100);

        std::ostringstream msg;
        msg << "El dado cayó en: " << roll;
        creature->Whisper(msg.str().c_str(), LANG_UNIVERSAL, player);

        // JACKPOT
        if (roll == 100)
        {
            player->AddItem(23247, 1);

            player->CastSpell(player, 31726);
            player->CastSpell(player, 47292);
            player->CastSpell(player, 44940);

            creature->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);

            std::string anuncio = "|cffFFD700[CASINO]|r " + player->GetName() + " ha ganado el JACKPOT!";
            ChatHandler(nullptr).SendWorldText(anuncio.c_str());

            ChatHandler(player->GetSession()).SendNotification("|cffFFD700¡¡JACKPOT!! Has ganado el premio especial.");

            CloseGossipMenuFor(player);
            return;
        }

        // ventaja para la casa
        if (roll >= 71)
        {
            uint32 emblema = ObtenerEmblemaAleatorio();

            player->AddItem(emblema, 1);

            player->CastSpell(player, 31726);
            player->CastSpell(player, 47292);

            creature->HandleEmoteCommand(EMOTE_ONESHOT_APPLAUD);

            ChatHandler(player->GetSession()).SendNotification("|cff00FF00¡Felicidades! Ganaste un emblema.");
        }
        else
        {
            creature->HandleEmoteCommand(EMOTE_ONESHOT_QUESTION);

            ChatHandler(player->GetSession()).SendNotification("|cffFF0000Mala suerte, perdiste tu apuesta.");
        }

        CloseGossipMenuFor(player);
    }
};

void AddSC_gamble_npc()
{
    new gamble_npc();
}