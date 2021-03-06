/***************************************************************************
    This file is part of Project Lemon
    Copyright (C) 2011 Zhipeng Jia

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#include "summarytree.h"
#include "addtestcaseswizard.h"
#include "settings.h"
#include "contest.h"
#include "task.h"
#include "testcase.h"

SummaryTree::SummaryTree(QWidget *parent) :
    QTreeWidget(parent)
{
    curContest = 0;
    addCount = 0;
    
    addTaskAction = new QAction(tr("Add a New Task"), this);
    addTestCaseAction = new QAction(tr("Add a Test Case"), this);
    addTestCasesAction = new QAction(tr("Add Test Cases ..."), this);
    deleteTaskAction = new QAction(tr("Delete Current Task"), this);
    deleteTestCaseAction = new QAction(tr("Delete Current Test Case"), this);
    addTaskKeyAction = new QAction(this);
    addTestCaseKeyAction = new QAction(this);
    deleteTaskKeyAction = new QAction(this);
    deleteTestCaseKeyAction = new QAction(this);
    
    addTaskKeyAction->setShortcutContext(Qt::WidgetShortcut);
    addTestCaseKeyAction->setShortcutContext(Qt::WidgetShortcut);
    deleteTaskKeyAction->setShortcutContext(Qt::WidgetShortcut);
    deleteTestCaseKeyAction->setShortcutContext(Qt::WidgetShortcut);
    addTaskKeyAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Insert));
    addTestCaseKeyAction->setShortcut(QKeySequence(Qt::Key_Insert));
    deleteTaskKeyAction->setShortcut(QKeySequence(Qt::Key_Delete));
    deleteTestCaseKeyAction->setShortcut(QKeySequence(Qt::Key_Delete));
    addTaskKeyAction->setEnabled(true);
    addTestCaseKeyAction->setEnabled(false);
    deleteTaskKeyAction->setEnabled(false);
    deleteTestCaseKeyAction->setEnabled(false);
    addAction(addTaskKeyAction);
    addAction(addTestCaseKeyAction);
    addAction(deleteTaskKeyAction);
    addAction(deleteTestCaseKeyAction);
    
    connect(addTaskAction, SIGNAL(triggered()),
            this, SLOT(addTask()));
    connect(addTestCaseAction, SIGNAL(triggered()),
            this, SLOT(addTestCase()));
    connect(addTestCasesAction, SIGNAL(triggered()),
            this, SLOT(addTestCases()));
    connect(addTaskKeyAction, SIGNAL(triggered()),
            this, SLOT(addTask()));
    connect(addTestCaseKeyAction, SIGNAL(triggered()),
            this, SLOT(addTestCase()));
    connect(deleteTaskAction, SIGNAL(triggered()),
            this, SLOT(deleteTask()));
    connect(deleteTestCaseAction, SIGNAL(triggered()),
            this, SLOT(deleteTestCase()));
    connect(deleteTaskKeyAction, SIGNAL(triggered()),
            this, SLOT(deleteTask()));
    connect(deleteTestCaseKeyAction, SIGNAL(triggered()),
            this, SLOT(deleteTestCase()));
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(selectionChanged()));
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
            this, SLOT(itemChanged(QTreeWidgetItem*)));
}

void SummaryTree::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        addTaskAction->setText(QApplication::translate("SummaryTree", "Add a New Task", 0));
        addTestCaseAction->setText(QApplication::translate("SummaryTree", "Add a Test Case", 0));
        addTestCasesAction->setText(QApplication::translate("SummaryTree", "Add Test Cases ...", 0));
        deleteTaskAction->setText(QApplication::translate("SummaryTree", "Delete Current Task", 0));
        deleteTestCaseAction->setText(QApplication::translate("SummaryTree", "Delete Current Test Case", 0));
        for (int i = 0; i < topLevelItemCount(); i ++) {
            QTreeWidgetItem *taskItem = topLevelItem(i);
            for (int j = 0; j < taskItem->childCount(); j ++) {
                taskItem->child(j)->setText(0, QApplication::translate("SummaryTree", "Test Case #%1", 0).arg(j + 1));
            }
        }
    }
}

void SummaryTree::setContest(Contest *contest)
{
    if (curContest) {
        QList<Task*> taskList = curContest->getTaskList();
        for (int i = 0; i <  taskList.size(); i ++)
            disconnect(taskList[i], SIGNAL(problemTitleChanged(QString)),
                       this, SLOT(titleChanged(QString)));
    }
    curContest = contest;
    if (! curContest) return;
    setEnabled(false);
    clear();
    QList<Task*> taskList = curContest->getTaskList();
    for (int i = 0; i < taskList.size(); i ++) {
        connect(taskList[i], SIGNAL(problemTitleChanged(QString)),
                this, SLOT(titleChanged(QString)));
        QTreeWidgetItem *newTaskItem = new QTreeWidgetItem(this);
        newTaskItem->setText(0, taskList[i]->getProblemTile());
        newTaskItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        for (int j = 0; j < taskList[i]->getTestCaseList().size(); j ++) {
            QTreeWidgetItem *newTestCaseItem = new QTreeWidgetItem(newTaskItem);
            newTestCaseItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            newTestCaseItem->setText(0, tr("Test Case #%1").arg(newTaskItem->childCount()));
        }
    }
    if (taskList.size() > 0) setCurrentItem(topLevelItem(0));
    setEnabled(true);
    emit currentItemChanged(0, 0);
}

void SummaryTree::setSettings(Settings *_settings)
{
    settings = _settings;
}

void SummaryTree::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *contextMenu = new QMenu(this);
    
    QTreeWidgetItem *curItem = currentItem();
    if (! curItem) {
        contextMenu->addAction(addTaskAction);
        contextMenu->exec(QCursor::pos());
        delete contextMenu;
        return;
    }
    
    int index = indexOfTopLevelItem(curItem);
    if (index != -1) {
        contextMenu->addAction(addTaskAction);
        contextMenu->addAction(deleteTaskAction);
        contextMenu->addSeparator();
        contextMenu->addAction(addTestCaseAction);
        contextMenu->addAction(addTestCasesAction);
        contextMenu->exec(QCursor::pos());
        delete contextMenu;
    } else {
        contextMenu->addAction(addTaskAction);
        contextMenu->addAction(deleteTaskAction);
        contextMenu->addSeparator();
        contextMenu->addAction(addTestCaseAction);
        contextMenu->addAction(addTestCasesAction);
        contextMenu->addAction(deleteTestCaseAction);
        contextMenu->exec(QCursor::pos());
        delete contextMenu;
    }
}

void SummaryTree::addTask()
{
    Task *newTask = new Task;
    newTask->setAnswerFileExtension(settings->getDefaultOutputFileExtension());
    curContest->addTask(newTask);
    newTask->refreshCompilerConfiguration(settings);
    connect(newTask, SIGNAL(problemTitleChanged(QString)),
            this, SLOT(titleChanged(QString)));
    QTreeWidgetItem *newItem = new QTreeWidgetItem(this);
    setCurrentItem(newItem);
    newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    newItem->setText(0, tr("Problem %1").arg(++ addCount));
    editItem(newItem);
}

void SummaryTree::addTestCase()
{
    QTreeWidgetItem *curItem = currentItem();
    if (indexOfTopLevelItem(curItem) == -1) {
        curItem = curItem->parent();
    }
    int index = indexOfTopLevelItem(curItem);
    Task *curTask = curContest->getTask(index);
    TestCase *newTestCase = new TestCase;
    newTestCase->setFullScore(settings->getDefaultFullScore());
    newTestCase->setTimeLimit(settings->getDefaultTimeLimit());
    newTestCase->setMemoryLimit(settings->getDefaultMemoryLimit());
    curTask->addTestCase(newTestCase);
    QTreeWidgetItem *newItem = new QTreeWidgetItem(curItem);
    newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    newItem->setText(0, tr("Test Case #%1").arg(curItem->childCount()));
    setCurrentItem(newItem);
}

void SummaryTree::addTestCases()
{
    QTreeWidgetItem *curItem = currentItem();
    if (indexOfTopLevelItem(curItem) == -1) {
        curItem = curItem->parent();
    }
    int index = indexOfTopLevelItem(curItem);
    Task *curTask = curContest->getTask(index);
    AddTestCasesWizard *wizard = new AddTestCasesWizard(this);
    wizard->setSettings(settings, curTask->getTaskType() == Task::Traditional);
    if (wizard->exec() == QDialog::Accepted) {
        QList<QStringList> inputFiles = wizard->getMatchedInputFiles();
        QList<QStringList> outputFiles = wizard->getMatchedOutputFiles();
        for (int i = 0; i < inputFiles.size(); i ++) {
            addTestCase();
            QTreeWidgetItem *curItem = currentItem();
            QTreeWidgetItem *parentItem = curItem->parent();
            int taskIndex = indexOfTopLevelItem(parentItem);
            int testCaseIndex = parentItem->indexOfChild(curItem);
            Task *curTask = curContest->getTask(taskIndex);
            TestCase *curTestCase = curTask->getTestCase(testCaseIndex);
            curTestCase->setFullScore(wizard->getFullScore());
            curTestCase->setTimeLimit(wizard->getTimeLimit());
            curTestCase->setMemoryLimit(wizard->getMemoryLimit());
            for (int j = 0; j < inputFiles[i].size(); j ++) {
                curTestCase->addSingleCase(inputFiles[i][j], outputFiles[i][j]);
            }
            setCurrentItem(parentItem);
            setCurrentItem(curItem);
        }
    }
    delete wizard;
}

void SummaryTree::deleteTask()
{
    if (QMessageBox::warning(this, tr("Lemon"), tr("Are you sure to delete this task?"),
                             QMessageBox::Yes, QMessageBox::Cancel) == QMessageBox::Cancel) {
        return;
    }
    QTreeWidgetItem *curItem = currentItem();
    if (indexOfTopLevelItem(curItem) == -1) {
        curItem = curItem->parent();
    }
    int index = indexOfTopLevelItem(curItem);
    if (index + 1 < topLevelItemCount()) {
        setCurrentItem(topLevelItem(index + 1));
    } else {
        if (index - 1 >= 0) {
            setCurrentItem(topLevelItem(index - 1));
        } else {
            setCurrentItem(0);
        }
    }
    delete curItem;
    curContest->deleteTask(index);
}

void SummaryTree::deleteTestCase()
{
    QTreeWidgetItem *curItem = currentItem();
    QTreeWidgetItem *parentItem = curItem->parent();
    int taskIndex = indexOfTopLevelItem(parentItem);
    int testCaseIndex = parentItem->indexOfChild(curItem);
    Task *curTask = curContest->getTask(taskIndex);
    delete curItem;
    curTask->deleteTestCase(testCaseIndex);
    for (int i = 0; i < parentItem->childCount(); i ++) {
        parentItem->child(i)->setText(0, tr("Test Case #%1").arg(i + 1));
    }
}

void SummaryTree::selectionChanged()
{
    if (! isEnabled()) return;
    QTreeWidgetItem *curItem = currentItem();
    if (! curItem) {
        addTestCaseKeyAction->setEnabled(false);
        deleteTaskKeyAction->setEnabled(false);
        deleteTestCaseKeyAction->setEnabled(false);
        return;
    }
    
    int index = indexOfTopLevelItem(curItem);
    if (index != -1) {
        addTestCaseKeyAction->setEnabled(true);
        deleteTaskKeyAction->setEnabled(true);
        deleteTestCaseKeyAction->setEnabled(false);
    } else {
        addTestCaseKeyAction->setEnabled(true);
        deleteTaskKeyAction->setEnabled(false);
        deleteTestCaseKeyAction->setEnabled(true);
    }
}

void SummaryTree::itemChanged(QTreeWidgetItem *item)
{
    int index = indexOfTopLevelItem(item);
    if (index != -1) {
        Task *curTask = curContest->getTask(index);
        curTask->setProblemTitle(item->text(0));
    }
}

void SummaryTree::titleChanged(const QString &title)
{
    QTreeWidgetItem *curItem = currentItem();
    if (curItem) curItem->setText(0, title);
}
