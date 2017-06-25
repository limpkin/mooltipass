package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

import mooltipass.automatedTest.pageObjects.AbstractPage;

public class Evernote extends AbstractPage{
	
	public Evernote (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	private boolean runMobile = false;
	@FindBy(xpath = "//a[contains(text(),'Sign In')]")
	private WebElement loginBtn;
	
	@FindBy(xpath = "//a[contains(text(),'Sign In') and @class='js-sign-in-menu click_tracking']")
	private WebElement loginBtnMobile;
	@FindBy(id = "username")
	private WebElement email;

	@FindBy(id = "password")
	private WebElement password;
	
	@FindBy(id = "loginButton")
	private WebElement submitLogin;
	
	
	@FindBy(xpath = "//div[contains(text(),'Log out')]")
	private WebElement logoutBtn;
	
	@FindBy(id = "gwt-debug-AccountMenuView-root")
	private WebElement dashBoard;
	
	

	@FindBy(id = "header-menu")
	private WebElement dashBoardMobile;
	
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		if(waitUntilAppears(loginBtn))
			loginBtn.click();
		else
		{
			waitUntilAppears(By.id("header-menu"));
			dashBoardMobile.click();
			loginBtnMobile.click();
			runMobile = true;
		}
	}
	
	public void submit(){
	submitLogin.click();
	}
	public void goTodDashboard()
	{
		waitUntilAppears(dashBoard);
		dashBoard.click();
		
	}
	public void logout(){
		logoutBtn.click();
	}
	
	public boolean checkLogin(){
		waitUntilAppears(dashBoard);
		return isElementPresent(By.id("gwt-debug-AccountMenuView-root"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("username"));
	}

}
