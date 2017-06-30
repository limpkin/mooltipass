package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Action;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;
public  class EBookers extends AbstractPage{
	
	public EBookers (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(id = "header-account-menu")
	private WebElement dashBoard;
	
	@FindBy(id = "header-account-signin")
	private WebElement loginBtn;
	
	@FindBy(id = "signin-loginid")
	private WebElement email;
	
	@FindBy(id= "signin-password")
	private WebElement password;
	
	@FindBy(id = "submitButton")
	private WebElement submitLogin;
	
	@FindBy(id = "header-account-signout")
	private WebElement logoutBtn;
	
	public void enterEmail(String value){
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		loginBtn.click();
	}
	
	public void submit(){
	submitLogin.click();
	}
	public void logout(){
		logoutBtn.click();
		sleep(2000);
	}
	public void goTodDashboard()
	{
		waitUntilAppears(dashBoard);
		dashBoard.click();
	}
	public boolean checkLogin(){	
		return isElementPresent(By.xpath("//a[contains(text(),'Bonjour Mooltipass')]"));
	}
	
	public boolean checkAtLoginPage(){
		waitUntilAppears(By.id("signin-loginid"));
		return isElementPresent(By.id("signin-loginid"));
	}
	
	
}
