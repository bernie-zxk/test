
{
int dosignal;

Pthread_mutex_lock(&nready.mutex);
dosignal = (nready.nready == 0);
nready.nready++;
Prhread_mutex_unlock(&nready.mutex);

if(dosignal)
{
    Prhread_cond_signal(&nread.cond);
}

}
