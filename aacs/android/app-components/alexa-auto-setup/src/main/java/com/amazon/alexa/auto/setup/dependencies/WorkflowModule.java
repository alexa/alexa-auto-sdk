package com.amazon.alexa.auto.setup.dependencies;

import android.content.Context;

import com.amazon.alexa.auto.setup.workflow.WorkflowNavigator;
import com.amazon.alexa.auto.setup.workflow.WorkflowProvider;

import java.lang.ref.WeakReference;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.inject.Named;
import javax.inject.Singleton;

import dagger.Module;
import dagger.Provides;

/**
 * Module to provide objects for Alexa setup workflow control.
 */
@Module
public class WorkflowModule {
    private static final String CONFIG_EXECUTOR_SERVICE = "config-exec-svc";

    @Provides
    @Singleton
    public WorkflowProvider provideWorkflowProvider(
            WeakReference<Context> context, @Named(CONFIG_EXECUTOR_SERVICE) ExecutorService executorService) {
        return new WorkflowProvider(context, executorService);
    }

    @Provides
    @Singleton
    public WorkflowNavigator provideWorkflowNavigator(
            WeakReference<Context> context, WorkflowProvider workflowProvider) {
        return new WorkflowNavigator(context, workflowProvider);
    }

    @Provides
    @Singleton
    @Named(CONFIG_EXECUTOR_SERVICE)
    public ExecutorService provideExecutorService() {
        return Executors.newSingleThreadExecutor();
    }
}
