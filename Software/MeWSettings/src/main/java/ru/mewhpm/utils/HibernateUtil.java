package ru.mewhpm.utils;

import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.Properties;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.boot.registry.StandardServiceRegistry;
import org.hibernate.boot.registry.StandardServiceRegistryBuilder;
import org.hibernate.cfg.Configuration;
import ru.mewhpm.dao.MeWConfig;

public class HibernateUtil {
    private static final Logger 
            log = Logger.getAnonymousLogger();
    
    private SessionFactory 
            currSF;
    
    private StandardServiceRegistry
            builder;
    
    private static HibernateUtil
            util = null;
    
    public static Session
            currSession = null;

    private HibernateUtil(String rootDBName, String uname, String upass) throws IOException {
        new File("." + File.separator + rootDBName + File.separator).mkdirs();
        final String dbURI = "jdbc:h2:." + File.separator + rootDBName + File.separator + "database;CIPHER=AES;";
        try {
            final Properties prop = new Properties();
            prop.setProperty("hibernate.connection.driver_class", "org.h2.Driver");
            prop.setProperty("hibernate.hbm2ddl.auto", "update");
            prop.setProperty("hibernate.connection.url", dbURI);
            prop.setProperty("hibernate.connection.username", uname);
            prop.setProperty("hibernate.connection.password", upass + " " + upass);
            prop.setProperty("dialect", "org.hibernate.dialect.H2Dialect");
            prop.setProperty("hibernate.show_sql", "true");
            // prop.setProperty("hibernate.format_sql", "true");
            
            final Configuration conf = new Configuration()
                    .addProperties(prop)
                    .addAnnotatedClass(MeWConfig.class)

                    .configure();
            
            builder = new StandardServiceRegistryBuilder().applySettings(conf.getProperties()).build();
            currSF = conf.buildSessionFactory(builder);
            
        } catch (Throwable ex) {
            log.log(Level.SEVERE, ex.getMessage());
            throw new IOException("HibernateUtil cannot init");
        }
    }
    
    public final SessionFactory getSessionFactory() {
        return currSF;
    }
    
    public final void disposeIt() {
        currSF.close();
        StandardServiceRegistryBuilder.destroy(builder); 
    }

    public static void init(String rootDBName, String uname, String upass) throws IOException {
        if (util == null) {
            util = new HibernateUtil(rootDBName, uname, upass);
            currSession = util.getSessionFactory().openSession();
        }
    }
    
    public static Session getNewSession() {
        return util.getSessionFactory().openSession();
    }
    
    public static Session getCurrentSession() {
        return currSession;
    }
    
    public static synchronized void sessionClose(Session currSession) {
        if (currSession != null) {
            currSession.close();
        } else 
            log.log(Level.WARNING, "Cannot close session, it's already closed.");
    }
    
    public static synchronized void beginTransaction(Session currSession) {
        if (currSession != null) {
            currSession.beginTransaction();
        } else 
            log.log(Level.WARNING, "Cannot begin transaction, session is already closed.");
    }
    
    public static synchronized void commitTransaction(Session currSession) {
        if (currSession != null) {
            currSession.getTransaction().commit();
        } else 
            log.log(Level.WARNING, "Cannot commit transaction, session is already closed.");
    }
    
    public static void dispose() {
        util.disposeIt();
    }
    
    public static void saveObject(Object o) {
        beginTransaction(getCurrentSession());
        getCurrentSession().save(o);
        commitTransaction(getCurrentSession());
    }
    
    public static void removeObject(Object o) {
        beginTransaction(getCurrentSession());
        getCurrentSession().delete(o);
        commitTransaction(getCurrentSession());
    }
    
    public static List getFullList(Class type) {
        return getCurrentSession()
                .createCriteria(type)
                .list();
    }
    
    public static List getPartOfList(Class type, int from, int to) {
        return getCurrentSession()
                .createCriteria(type)
                .setFirstResult(from)
                .setMaxResults(to)
                .list();
    }
}
