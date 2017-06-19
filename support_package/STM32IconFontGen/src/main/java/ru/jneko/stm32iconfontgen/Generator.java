package ru.jneko.stm32iconfontgen;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.io.File;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import javafx.application.Application;
import static javafx.application.Application.launch;
import javafx.application.Platform;
import javafx.stage.Stage;
import javax.imageio.ImageIO;

public class Generator  extends Application {
    public static final String 
            FONT_NAME = "MEW_ICONS";
    
    public static final int
            FONT_HEIGHT = 24,
            FONT_WIDTH  = 24,
            FONT_BPP    = 1;
    
    private String bitmapToHex(BufferedImage bi, char c) {
        final StringBuilder output = new StringBuilder();
        output.append("static const u8 icon_").append(Integer.toHexString(c).toUpperCase()).append("[").append((FONT_HEIGHT*FONT_WIDTH)/8).append("]\t= {"); 
        
        int currBit = 0, currChar = 0;
        
        for (int y=0; y<FONT_HEIGHT; y++) {
            for (int x=0; x<FONT_HEIGHT; x++) {
                if (currBit > 7) {
                    currBit = 0;
                    final String bOut = "00" + Integer.toHexString(currChar).toUpperCase();
                    output.append("0x").append(bOut.substring(bOut.length()-2, bOut.length())).append(", ");
                    currChar = 0;
                }
                
                if (bi.getRGB(x, y) == Color.BLACK.getRGB()) 
                    currChar |= (1 << currBit);
                
                currBit++;
            }
        }
        
        output.append("};\n");
        return output.toString();
    }
    
    @Override
    public void start(Stage stage) throws Exception {
        final StringBuilder output = new StringBuilder();
        output
                .append("// STM32 iconpack generator\n")
                .append("#ifndef __FONT_STM32_").append(FONT_NAME).append("__\n")
                .append("#define __FONT_STM32_").append(FONT_NAME).append("__\n\n")
                .append("#define ").append(FONT_NAME).append("_FONT_HEIGHT ").append(FONT_HEIGHT).append("\n")
                .append("#define ").append(FONT_NAME).append("_FONT_WIDTH ").append(FONT_WIDTH).append("\n")
                .append("#define ").append(FONT_NAME).append("_FONT_BPP ").append(FONT_BPP).append("\n\n");
        
        final BufferedImage bi = new BufferedImage(FONT_WIDTH, FONT_HEIGHT, BufferedImage.TYPE_INT_ARGB);
        final Graphics g = bi.getGraphics();
     
        final Font f = Font.createFont(Font.TRUETYPE_FONT, new File("./font.ttf"));
        final Font font = f.deriveFont(Font.PLAIN, 24);
        g.setFont(font);
        char chars[] = { 0 };
        
        new File("./icons/").mkdirs();
        
        for (char c = 0xe000; c <= 0xF000; c++) {
            if (f.canDisplay(c)) {
                chars[0] = c;
                g.setColor(Color.WHITE); 
                g.fillRect(0, 0, FONT_WIDTH, FONT_HEIGHT);
                g.setColor(Color.BLACK); 
                g.drawChars(chars, 0, 1, 0, FONT_HEIGHT);
                
                output.append(bitmapToHex(bi, c));

                ImageIO.write(bi, "png", new File("./icons/0x"+Integer.toHexString(c)+".png"));
            }
        }
        
        output.append("\n#endif\n");
        Files.write(FileSystems.getDefault().getPath(new File("./font.c").getAbsolutePath()), output.toString().getBytes());
        
        Platform.exit();
    }

    public static void main(String[] args) {
        launch(args);
    }
}
