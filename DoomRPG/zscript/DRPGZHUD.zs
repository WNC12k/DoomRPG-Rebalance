class ZHUD : EventHandler
{
    private transient CVar pack;
    private transient CVar multi_hud_x;
    private transient CVar multi_hud_y;
    
    private TextureID Tex[2];
    
    override void OnRegister()
    {
        pack = CVar.GetCVar('drpg_multi_data');
        multi_hud_x = CVar.FindCVar('drpg_multiplayer_x');
        multi_hud_y = CVar.FindCVar('drpg_multiplayer_y');
        
        static const string NAMES[] = {
            "MPFill", // Health
            "MPFill2" // Shield
        };
        
        for (int i = 0; i < NAMES.Size(); i++)
        {
            Tex[i] = TexMan.CheckForTexture(NAMES[i], TexMan.Type_Any);
        }
    }
    
    override void RenderUnderlay(RenderEvent e)
    {
        if (!pack)
            return;
        
        String data = pack.GetString();
        data.StripRight("|");
        
        if (data.Length() > 0)
        {
            float scale = min(Screen.GetWidth() / 1920.0, Screen.GetHeight() / 1080.0);
            int x = multi_hud_x.GetInt() / 1920.0 * Screen.GetWidth();
            int y = multi_hud_y.GetInt() / 1080.0 * Screen.GetHeight();
            int skip = 0;
            
            Array<String> unpack;
            data.Split(unpack, "|");
            
            for (int i = 0; i < unpack.Size(); i += 4)
            {
                if (unpack[i + 1] == "0")
                {
                    skip++;
                    continue;
                }
                
                int offset = (i - skip) / 4 * 45;
                
                Screen.DrawText(NewSmallFont, Font.CR_WHITE,
                    x / scale, y / scale + offset,
                    players[i / 4].GetUserName(),
                    DTA_VirtualWidth, 1920,
                    DTA_VirtualHeight, 1080,
                    DTA_KeepRatio, true,
                    DTA_Color, Color(players[i].GetColor() | 0xFF000000));
                
                if (unpack[i].ToInt() >= 0)
                {
                    Screen.DrawTexture(Tex[0], false,
                        x, y + (20 + offset) * scale,
                        DTA_DestWidth, int(200 * min(unpack[i].ToDouble() / unpack[i + 1].ToDouble(), 1) * scale),
                        DTA_DestHeight, int(20 * scale),
                        DTA_Alpha, 0.8);
                }
                
                if (unpack[i + 2] != "0")
                {
                    Screen.DrawTexture(Tex[1], false,
                        x, y + (20 + offset) * scale,
                        DTA_DestWidth, int(200 * min(unpack[i + 2].ToDouble() / unpack[i + 3].ToDouble(), 1) * scale),
                        DTA_DestHeight, int(20 * scale),
                        DTA_Alpha, 0.5);
                    
                    Screen.DrawText(NewSmallFont, Font.CR_LIGHTBLUE,
                        x / scale + 150, y / scale + offset,
                        unpack[i + 2].."/"..unpack[i + 3],
                        DTA_VirtualWidth, 1920,
                        DTA_VirtualHeight, 1080,
                        DTA_KeepRatio, true);
                }
                else
                {
                    Screen.DrawText(NewSmallFont, Font.CR_BRICK,
                        x / scale + 150, y / scale + offset,
                        unpack[i].."/"..unpack[i + 1],
                        DTA_VirtualWidth, 1920,
                        DTA_VirtualHeight, 1080,
                        DTA_KeepRatio, true);
                }
            }
        }
    }
}
