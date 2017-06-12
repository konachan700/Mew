/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.uifontgen;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.io.File;
import javafx.application.Application;
import javafx.application.Platform;
import javafx.stage.Stage;
import javax.imageio.ImageIO;

/**
 *
 * @author misaki
 */
public class UIFontGen extends Application {
    
    
    
    
    
    
    @Override
    public void start(Stage stage) throws Exception {
        
        final BufferedImage bi = new BufferedImage(100, 100, BufferedImage.TYPE_INT_ARGB);
        final Graphics g = bi.getGraphics();
        
        final Font f = Font.createFont(Font.TRUETYPE_FONT, new File("./Hack-Regular.ttf"));
        //final Font font = f.deriveFont(Font.PLAIN, 16f);
        
        Font font = new Font ("Courier New", 1, 12);
        final Rectangle2D r2d = font.getStringBounds("W", ((Graphics2D) g).getFontRenderContext());
        
        System.out.println(r2d.getBounds().height + "  w"+r2d.getBounds().width);
        
        g.setFont(font);
        
                        g.setColor(Color.WHITE); 
                g.fillRect(0, 0, 100, 100);
                g.setColor(Color.BLACK); 
                g.drawChars("sdWEWsdfg".toCharArray(), 0, 8, 0, 50);
        
        ImageIO.write(bi, "png", new File("./out1.png"));
        
        
        Platform.exit();
    }
    
    public static void main(String[] args) {
        launch(args);
    }
}
