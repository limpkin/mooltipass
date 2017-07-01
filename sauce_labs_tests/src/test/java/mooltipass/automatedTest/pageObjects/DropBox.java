package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Action;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;
public class DropBox extends AbstractPage{
	
	public DropBox (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(xpath = "//a[text()='Sign in']")
	private WebElement loginBtn;
	
	@FindBy(xpath = "//input[@name='login_email']")
	private WebElement email;

	@FindBy(xpath = "//input[@name='login_password']")
	private WebElement password;
	@FindBy(xpath = "//form[@action='/ajax_login']//button[@class='login-button button-primary']//div[@class='sign-in-text']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//button[@aria-label='Account menu']")
	private WebElement dashBoard;
	
	@FindBy(xpath = "//a[@href='https://www.dropbox.com/logout']")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//a[@aria-label='Close']")
	private WebElement popUp;
	
			
	public void enterEmail(String value){
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){

		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void goToLogin(){
		waitUntilAppears(loginBtn);		
		loginBtn.click();
	}
	
	public void submit(){
	submitLogin.click();
	}
	public void goTodDashboard()
	{
		if(isElementPresent(By.xpath("//a[@aria-label='Close']")) && popUp.isDisplayed() )
			popUp.click();
		waitUntilAppears(dashBoard);
		dashBoard.click();
		
	}
	public void logout(){
		logoutBtn.click();
	}
	
	public boolean checkLogin(){

		waitUntilAppears(By.xpath("//button[@aria-label='Account menu']"));
		return isElementPresent(By.xpath("//button[@aria-label='Account menu']"));
	}
	public boolean checkAtLoginPage(){
		return isElementPresent(By.id("sign-in"));
	}

}