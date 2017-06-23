import web
import json
import sys
import MySQLdb as mdb
import logging
import subprocess
import urllib2
#import requests

from threading import Timer
from time import sleep


class RepeatedTimer(object):
    def __init__(self, interval, function, *args, **kwargs):
        self._timer     = None
        self.function   = function
        self.interval   = interval
        self.args       = args
        self.kwargs     = kwargs
        self.is_running = False
        self.start()

    def _run(self):
        self.is_running = False
        self.start()
        self.function(*self.args, **self.kwargs)

    def start(self):
        if not self.is_running:
            self._timer = Timer(self.interval, self._run)
            self._timer.start()
            self.is_running = True

    def stop(self):
        self._timer.cancel()
        self.is_running = False


urls = (
    '/', 'index',
    '/json', 'data_per_post'
)

class data_per_post:
    def GET(self):
        return "Hello, world! data"

    def POST(self):
        logger = logging.getLogger("process-POST")
        try:
            data = json.loads(web.data())
            print data
#            json.dumps(data)
##            data = urlparse.parse_qs(r)
#            print json.loads(r['payload'][0])
#            json_value = json.loads(data)
            print len(data)
#            if len(data)==3:

#            led=data["sensorid"]
#            count=data["count"]
#            basetta=data["basetta"]
# {"lamp":50,"powerenabled":0,"alarmenabled":0,"buzzerenabled":0,"alarmtime":"09:30","powertime":"23:30"}
# {"ip":"10.42.0.61","pulsante":0,"stato":9999,"valore":50,"slenabled":0,"alenabled":0,"buenabled":0,"alarmtime":"09:30","sleeptime":"23:30","powered":1}
            basetta=data["ip"]  
            
            evento=data["evento"]
            stato=data["stato"]   
            valore=data["valore"]
            pulsante=data["pulsante"]

#            buzzerenabled=data["buenabled"]
#            alarmenabled=data["alenabled"]
#            sleepenabled=data["slenabled"]
#            sleeptime=data["sleeptime"]                
#            alarmtime=data["alarmtime"]                
#            powered=data["powered"]    

#define LIGHT 1
#define ALARM 2
#define POWER 3
#define BUZZER 4
#define SLEEP 5
#define TEMPERATURE 6
#define SWITCH 7
#define PIR 8
#define AMPERE 9
#define xxx 10
#define yyy 11
#define SEND_PRESENCE 9999
#define CONFIGURATION 9998

            con = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione'); 
            cur = con.cursor(mdb.cursors.DictCursor)
            cur1 = con.cursor(mdb.cursors.DictCursor)
            if evento == 9999:
                print("Received presence")
                sql="select elemento, bistate from illuminazione.impianto where matricola='%s';" % (basetta)
                cur.execute(sql)
                if cur.rowcount==0:
                    parts = basetta.split('.')
                    matricola=(int(parts[2])*256+int(parts[3]))
                    sql="insert into illuminazione.impianto (matricola, attivo) values('%s',1);" % (basetta)
                    cur1.execute(sql)
                    con.commit()
                    
#                    sql="insert into illuminazione.elemento (elemento,critica,attiva,matricola,tipologia) values ('%s',0,0,'%s',%d);" % (elemento, matricola, pulsante)
#                    cur.execute(sql)
#                    con.commit()
                else:
                    rows = cur.fetchall()
                    for row in rows:
                        elemento=row['elemento']
                        sql="update illuminazione.impianto set attivo=1 where matricola='%s';" % (basetta)
                        cur1.execute(sql)
                        con.commit()
                print("Presence of %s" % (basetta))
                return
            if evento == 1:
                print "i am in show"
                print "Valori Basetta '%s' stato '%s' evento '%s' valore '%s' pulsante '%s'" % (basetta, stato, evento, valore, pulsante)
                logger.debug("Light management")
                sql="select e.elemento, i.bistate from illuminazione.elemento e, illuminazione.impianto i \
                     where e.elemento=i.elemento \
                     and current_time() between concat(e.ora_inizio,':00') and concat(e.ora_fine,':59') \
                     and i.matricola='%s' \
                     and i.pulsante=%s \
                     and e.attivo=1;" % (basetta, pulsante)
                with con:
                    cur = con.cursor(mdb.cursors.DictCursor)
                    cur.execute(sql)
                    if cur.rowcount>0:
                        rows = cur.fetchall()
                        for row in rows:
                            elemento=row['elemento']
                            bistate=row['bistate']
                            #
                            # Inserire verifica sul tipo di dato ricevuto: solo da pulsantiera eseguire il codice qui sotto
                            #
                            if (bistate==1):
                                sql="update illuminazione.elemento e set valore=abs(valore-1) where elemento=%s \
                                     and current_time() between concat(e.ora_inizio,':00') and concat(e.ora_fine,':59'); " % (elemento)
                                cur1.execute(sql)
                                con.commit()                                    
                                sql="update illuminazione.elemento e set valore=0 where elemento=%s \
                                     and current_time() not between concat(e.ora_inizio,':00') and concat(e.ora_fine,':59'); " % (elemento)
                                cur1.execute(sql)
                                con.commit()                                    
                            if (bistate==0):
                                sql="update illuminazione.elemento e set valore=%s where elemento=%s \
                                     and current_time() between concat(e.ora_inizio,':00') and concat(e.ora_fine,':59'); " % (valore, elemento)
                                cur1.execute(sql)
                                con.commit()
                                sql="update illuminazione.elemento e set valore=0 where elemento=%s \
                                     and current_time() not between concat(e.ora_inizio,':00') and concat(e.ora_fine,':59'); " % (elemento)
                                cur1.execute(sql)
                                con.commit()

                            sql="select i.matricola, i.pulsante, e.valore from illuminazione.elemento e, illuminazione.impianto i \
                                 where current_time() between concat(e.ora_inizio,':00') and concat(e.ora_fine,':59') \
                                 and e.elemento='%s' \
                                 and e.attivo=1;" % (elemento)

                            cur1.execute(sql)
                            if cur1.rowcount>0:
                                rows = cur1.fetchall()
                                for row in rows:
                                    ip=row["matricola"]
                                    pulsante=row["pulsante"]
                                    valore=row["valore"]
                                    logger.debug("Check Light management IP %s valore %s pulsante %s " % (ip,valore,pulsante))
                                    req = 'http://%s/api/output?valore=%s&pulsante=%s' % (ip,valore,pulsante)
                                    urllib2.urlopen(req).read()
                    else:
                        logger.debug("Received various data")
                con.close()
                return
        except IOError as e:
            print "I/O error({0}): {1}".format(e.errno, e.strerror)
        except ValueError:
            print "Could not convert data to an integer."
        except:
            print "Unexpected error:", sys.exc_info()[0]
            raise

#        except Error(e):
#            print e

class index:
    def GET(self):
        return "Hello, world!"
    
def send_conf(text):
    con = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione');
    cur = con.cursor(mdb.cursors.DictCursor)
    logger.debug("Send configuration")
    sql="select i.matricola, i.pulsante, e.valore from illuminazione.elemento e, illuminazione.impianto i \
         where current_time() between concat(e.ora_inizio,':00') and concat(e.ora_fine,':59') \
         and e.elemento=i.elemento \
         and e.attivo=1;"
    cur.execute(sql)
    if cur.rowcount>0:
        rows = cur.fetchall()
        for row in rows:
            ip=row["matricola"]
            pulsante=row["pulsante"]
            valore=row["valore"]
            logger.debug("Check send_conf IP %s valore %s pulsante %s " % (ip,valore,pulsante))
            req = 'http://%s/api/output?valore=%s&pulsante=%s' % (ip,valore,pulsante)
            urllib2.urlopen(req).read()
    con.close()

def presence(text):
    con = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione');
    cur = con.cursor(mdb.cursors.DictCursor)
    logger.debug("Check presence")
    sql="SELECT matricola FROM illuminazione.impianto where attivo=1;"
    cur.execute(sql)
    if cur.rowcount>0:
        rows = cur.fetchall()
        for row in rows:
            ip=row['matricola']
            logger.debug("Check IP %s" % (ip))
            try:
                if subprocess.call("ping -c 1 %s" % ip , shell=True) == 0:
                    print ("host appears to be up")
                else:
                    print ("host appears to be down")
                    con1 = mdb.connect('localhost', 'illuminazione', 'illuminazione', 'illuminazione');
                    cur1 = con1.cursor(mdb.cursors.DictCursor)
                    sql="update illuminazione.matricole set attivo=0 where matricola='%s';" % (ip)
                    cur1.execute(sql)
                    con1.commit()  
                    con1.close()
            except:
                print ("Ping Error:", e)
    con.close()


class MyApplication(web.application):
    def run(self, port=5008, *middleware):
        func = self.wsgifunc(*middleware)
        return web.httpserver.runsimple(func, ('10.42.0.1', port))


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    logger = logging.getLogger("server")
    logger.info("starting...")

    rt = RepeatedTimer(600, send_conf, "Send conf") # it auto-starts, no need of rt.start()
    rt = RepeatedTimer(600, presence, "Presence") # it auto-starts, no need of rt.start()

    try:
        app = MyApplication(urls, globals())
        app.run()

    finally:
        rt.stop() # better in a try/finally block to make sure the program ends!
        logger.info("Shutting down")
        if con:    
            con.close()
           
    logger.info("All done")
