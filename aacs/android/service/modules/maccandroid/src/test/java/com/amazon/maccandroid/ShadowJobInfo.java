package com.amazon.maccandroid;

import android.app.job.JobInfo;
import android.app.job.JobService;

import org.mockito.Mockito;
import org.robolectric.annotation.Implementation;
import org.robolectric.annotation.Implements;
import org.robolectric.annotation.RealObject;

@Implements(JobInfo.Builder.class)
public class ShadowJobInfo {
    @RealObject
    protected JobInfo.Builder jobInfoBuilder;
    protected JobInfo jobInfo;

    public void __constructor__(int jobid, JobService jobService) {}

    @Implementation
    public JobInfo.Builder setPriority(int priority) {
        return jobInfoBuilder;
    }

    @Implementation
    public JobInfo.Builder setFlags(int flags) {
        return jobInfoBuilder;
    }

    @Implementation
    public JobInfo.Builder setPeriodic(long intervalMillis) {
        return jobInfoBuilder;
    }

    @Implementation
    public JobInfo build() {
        return Mockito.mock(JobInfo.class);
    }
}
